//========================================================================
//
// Form.cc
//
// Copyright 2006 Julien Rebetez
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <string.h>
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Form.h"
#include "XRef.h"
#include "PDFDocEncoding.h"
#include "Annot.h"
#include "Catalog.h"

//return a newly allocated char* containing an UTF16BE string of size length
static char* pdfDocEncodingToUTF16 (GooString* orig, int* length)
{
  //double size, a unicode char takes 2 char, add 2 for the unicode marker
  *length = 2+2*orig->getLength();
  char *result = new char[(*length)];
  char *cstring = orig->getCString();
  //unicode marker
  result[0] = 0xfe;
  result[1] = 0xff;
  //convert to utf16
  for(int i=2,j=0; i<(*length); i+=2,j++) {
    Unicode u = pdfDocEncoding[(unsigned int)((unsigned char)cstring[j])]&0xffff;
    result[i+1] = *(char*)(&u);
    result[i] = *(1+(char*)(&u));
  }
  return result;
}



FormWidget::FormWidget(XRef *xrefA, Object *aobj, unsigned num, Ref aref) 
{
  Object obj1, obj2;
  ref = aref;
  double t;
  ID = 0;
  fontSize = 0.0;
  childNum = num;
  xref = xrefA;
  aobj->copy(&obj);
  type = formUndef;
  Dict *dict = obj.getDict();

  if (!dict->lookup("Rect", &obj1)->isArray()) {
    error(-1, "Annotation rectangle is wrong type");
    goto err2;
  }
  if (!obj1.arrayGet(0, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  x1 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(1, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  y1 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(2, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  x2 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(3, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  y2 = obj2.getNum();
  obj2.free();
  obj1.free();
  //swap coords if needed
  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }
  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }
  
  err1:
    obj2.free();  
  err2:
    obj1.free();
}

FormWidget::FormWidget(FormWidget *dest)
{
  x1 = dest->x1;
  y1 = dest->y1;
  x2 = dest->x2;
  y2 = dest->x2;

  type = dest->type;

}

FormWidget::~FormWidget()
{

}

int FormWidget::encodeID (unsigned pageNum, unsigned fieldNum)
{
  return (pageNum << 4*sizeof(unsigned)) + fieldNum;
}

void FormWidget::decodeID (unsigned id, unsigned* pageNum, unsigned* fieldNum)
{
  *pageNum = id >> 4*sizeof(unsigned);
  *fieldNum = (id << 4*sizeof(unsigned)) >> 4*sizeof(unsigned);
}

FormWidgetButton::FormWidgetButton (XRef *xrefA, Object *aobj, unsigned num, Ref ref, FormFieldButton *p) : FormWidget(xrefA, aobj, num, ref)
{
  parent = p;
  type = formButton;
  onStr = NULL;
  Dict *dict = obj.getDict();
  Object obj1;
  state = gFalse;
  siblingsID = NULL;
}

FormWidgetButton::~FormWidgetButton ()
{
  if (siblingsID)
    gfree(siblingsID);
}

void FormWidgetButton::setState (GBool astate, GBool calledByParent)
{
  //the state modification may be denied by the parent. e.g we don't want to let the user put all combo boxes to false
  if (!calledByParent) { //avoid infinite recursion
    if (!parent->setState(childNum, astate)) {
      return;
    }
  }
  state = astate;
  //update appearance
  char *offStr = "Off";
  Object *obj1 = new Object();
  obj1->initName(state?onStr:offStr);
  obj.getDict()->set("V", obj1);
  obj1 = new Object();
  obj1->initName(state?onStr:offStr);
  //modify the Appearance State entry as well
  obj.getDict()->set("AS", obj1);

  //notify the xref about the update
  xref->setModifiedObject(&obj, ref);
}

bool FormWidgetButton::isReadOnly() const
{
  return parent->isReadOnly();
}

void FormWidgetButton::loadDefaults ()
{
  Dict *dict = obj.getDict();
  Object obj1;

  //pushButtons don't have state
  if (parent->getButtonType() != formButtonPush ){
    //find the name of the state in the AP dictionnary (/Yes, /Off)
    //The reference say the Off state, if it existe, _must_ be stored in the AP dict under the name /Off
    //The "on" state may be stored under any other name
    if (dict->lookup("AP", &obj1)->isDict()) {
      Dict *tmpDict = obj1.getDict();
      int length = tmpDict->getLength();
      for(int i=0; i<length; i++) {
        Object obj2;
        tmpDict->getVal(i, &obj2);
        if (obj2.isDict()) {
          Dict *tmpDict2 = obj2.getDict();
          int length2 = tmpDict2->getLength();
          for(int j=0; j<length2; j++) {
            Object obj3;
            tmpDict2->getVal(j, &obj3);
            char *key = tmpDict2->getKey(j);
            if(strcmp(key, "Off")) { //if we don't have Off, we have the name of the "on" state
              onStr = strdup(key);
            }
            obj3.free();
          }
        } else if (obj2.isStream()) {
          Stream *str = obj2.getStream();
          Dict *tmpDict2 = str->getDict();
          Object obj3;
          tmpDict2->lookup("Length", &obj3);
          int c;
          onStr = "D"; 
        }
        obj2.free();
      }
    }
    obj1.free();

    //We didn't found the "on" state for the button
    if (!onStr) {
      error(-1, "FormWidgetButton:: unable to find the on state for the button\n");
    }
  }

  if (dict->lookup("V", &obj1)->isName()) {
    char *s = obj1.getName();
    if(strcmp(s, "Off")) {
      //state = gTrue;
      setState(gTrue);
    }
  } else if (obj1.isArray()) { //handle the case where we have multiple choices
    error(-1, "FormWidgetButton:: multiple choice isn't supported yet\n");
  }
}

GBool FormWidgetButton::getState ()
{
  return state;
}

void FormWidgetButton::setNumSiblingsID (int i)
{ 
  numSiblingsID = i; 
  siblingsID = (unsigned*)greallocn(siblingsID, numSiblingsID, sizeof(unsigned));
}


FormWidgetText::FormWidgetText (XRef *xrefA, Object *aobj, unsigned num, Ref ref, FormFieldText *p) : FormWidget(xrefA, aobj, num, ref)
{
  parent = p;
  type = formText;
  Dict *dict = aobj->getDict();
}

void FormWidgetText::loadDefaults ()
{
  Dict *dict = obj.getDict();
  Object obj1;

  if (dict->lookup("V", &obj1)->isString()) {
    if (obj1.getString()->hasUnicodeMarker()) {
      if (obj1.getString()->getLength() <= 2) {
      } else {
        parent->setContentCopy(obj1.getString());
      }
    } else {
      if (obj1.getString()->getLength() > 0) {
        //non-unicode string -- assume pdfDocEncoding and try to convert to UTF16BE
        int tmp_length;
        char* tmp_str = pdfDocEncodingToUTF16(obj1.getString(), &tmp_length);
        GooString* str1 = new GooString(tmp_str, tmp_length);
        parent->setContentCopy(str1);
        delete str1;
      }
    }
  }
  obj1.free();

}

GooString* FormWidgetText::getContent ()
{
  return parent->getContent(); 
}

GooString* FormWidgetText::getContentCopy ()
{
  return parent->getContentCopy();
}
  
bool FormWidgetText::isMultiline () const 
{ 
  return parent->isMultiline(); 
}

bool FormWidgetText::isPassword () const 
{ 
  return parent->isPassword(); 
}

bool FormWidgetText::isFileSelect () const 
{ 
  return parent->isFileSelect(); 
}

bool FormWidgetText::noSpellCheck () const 
{ 
  return parent->noSpellCheck(); 
}

bool FormWidgetText::noScroll () const 
{ 
  return parent->noScroll(); 
}

bool FormWidgetText::isComb () const 
{ 
  return parent->isComb(); 
}

bool FormWidgetText::isRichText () const 
{ 
  return parent->isRichText(); 
}

bool FormWidgetText::isReadOnly () const
{
  return parent->isReadOnly();
}

void FormWidgetText::setContent(GooString* new_content)
{
  if (isReadOnly()) {
    error(-1, "FormWidgetText::setContentCopy called on a read only field\n");
    return;
  }

  if (new_content == NULL) {
    parent->setContentCopy(NULL);
  } else {
    //append the unicode marker <FE FF> if needed
    if (!new_content->hasUnicodeMarker()) {
      new_content->insert(0, 0xff);
      new_content->insert(0, 0xfe);
    }
    
    GooString *cont = new GooString(new_content);
    parent->setContentCopy(cont);
    Object *obj1 = new Object();
    obj1->initString(cont);
    obj.getDict()->set("V", obj1);
    //notify the xref about the update
    xref->setModifiedObject(&obj, ref);

  }
}

FormWidgetChoice::FormWidgetChoice(XRef *xrefA, Object *aobj, unsigned num, Ref ref, FormFieldChoice *p) : FormWidget(xrefA, aobj, num, ref)
{
  parent = p;
  type = formChoice;
}

void FormWidgetChoice::loadDefaults ()
{
  Dict *dict = obj.getDict();
  Object obj1;
  if (dict->lookup("Opt", &obj1)->isArray()) {
    Object obj2;
    parent->_setNumChoices(obj1.arrayGetLength());
    parent->_createChoicesTab();
    for(int i=0; i<parent->getNumChoices(); i++) {
      obj1.arrayGet(i, &obj2);
      if(obj2.isString()) {
        parent->_setChoiceExportVal(i, obj2.getString()->copy());
        parent->_setChoiceOptionName(i, obj2.getString()->copy());
      } else if (obj2.isArray()) { // [Export_value, Displayed_text]
        Object obj3,obj4;
        if (obj2.arrayGetLength() < 2) {
          error(-1, "FormWidgetChoice:: invalid Opt entry -- array's length < 2\n");
          parent->_setChoiceExportVal(i, new GooString(""));
          parent->_setChoiceOptionName(i, new GooString(""));
          continue;
        }
        obj2.arrayGet(0, &obj3);
        obj2.arrayGet(0, &obj4);
        parent->_setChoiceExportVal(i, obj3.getString()->copy());
        parent->_setChoiceOptionName(i, obj4.getString()->copy());
        obj3.free();
        obj4.free();
      } else {
        error(-1, "FormWidgetChoice:: invalid Opt entry\n");
      }
      obj2.free();
    }
  } else {
    //empty choice
  }
  obj1.free();

  bool* tmpCurrentChoice = new bool[parent->getNumChoices()];
  memset(tmpCurrentChoice, 0, sizeof(bool)*parent->getNumChoices());

  //find default choice
  if (dict->lookup("V", &obj1)->isString()) {  
    for(int i=0; i<parent->getNumChoices(); i++) {
      if (parent->getChoice(i)->cmp(obj1.getString()) == 0) {
        tmpCurrentChoice[i] = true;
        break;
      }
    }
  } else if (obj1.isArray()) {
    for(int i=0; i<obj1.arrayGetLength(); i++) {
      Object obj2;
      obj1.arrayGet(i, &obj2);
      for(int j=0; j<parent->getNumChoices(); j++) {
        if (parent->getChoice(j)->cmp(obj2.getString()) == 0) {
          tmpCurrentChoice[i] = true;
        }
      }

      obj2.free();
    }
  }
  obj1.free();

  //convert choice's human readable strings to UTF16
  //and update the /Opt dict entry to reflect this change
  #ifdef UPDATE_OPT
  Object *objOpt = new Object();
  objOpt->initArray(xref);
  #endif
  for(int i=0; i<parent->getNumChoices(); i++) {
        if (parent->getChoice(i)->hasUnicodeMarker()) { //string already in UTF16, do nothing

        } else { //string in pdfdocencoding, convert to UTF16
          int len;
          char* buffer = pdfDocEncodingToUTF16(parent->getChoice(i), &len);
          parent->getChoice(i)->Set(buffer, len);
          delete [] buffer;
        }
        #ifdef UPDATE_OPT
        Object *obj2 = new Object();
        obj2->initString(choices[i]);
        objOpt->getArray()->add(obj2);  
        #endif
  }
  //set default choice now that we have UTF16 strings
  for (int i=0; i<parent->getNumChoices(); i++) {
    if (tmpCurrentChoice[i])
      parent->select(i);
  }
  #ifdef UPDATE_OPT
  obj.getDict()->set("Opt", objOpt);
  xref->setModifiedObject(&obj, ref); 
  #endif
}

FormWidgetChoice::~FormWidgetChoice()
{
}

void FormWidgetChoice::_updateV ()
{
  Object *obj1 = new Object();
  //this is an editable combo-box with user-entered text
  if (hasEdit() && parent->getEditChoice()) { 
    obj1->initString(new GooString(parent->getEditChoice()));
  } else {
    int numSelected = parent->getNumSelected();
    if (numSelected == 0) {
      obj1->initString(new GooString(""));
    } else if (numSelected == 1) {
      for(int i=0; i<parent->getNumChoices(); i++) {
        if (parent->isSelected(i)) {
          obj1->initString(new GooString(parent->getExportVal(i)));
          break;
        }
      }
    } else {
      obj1->initArray(xref);
      for(int i=0; i<parent->getNumChoices(); i++) {
        if (parent->isSelected(i)) {
          Object* obj2 = new Object();
          obj2->initString(new GooString(parent->getExportVal(i)));
          obj1->arrayAdd(obj2);
        }
      }
    }
  }
  obj.getDict()->set("V", obj1);
  //notify the xref about the update
  xref->setModifiedObject(&obj, ref);
}

bool FormWidgetChoice::_checkRange (int i)
{
  if (i < 0 || i >= parent->getNumChoices()) {
    error(-1, "FormWidgetChoice::_checkRange i out of range : %i", i);
    return false;
  } 
  return true;
}

void FormWidgetChoice::select (int i)
{
  if (isReadOnly()) {
    error(-1, "FormWidgetChoice::select called on a read only field\n");
    return;
  }
  if (!_checkRange(i)) return;
  parent->select(i);
  _updateV();
}

void FormWidgetChoice::toggle (int i)
{
  if (isReadOnly()) {
    error(-1, "FormWidgetChoice::toggle called on a read only field\n");
    return;
  }
  if (!_checkRange(i)) return;
  parent->toggle(i);
  _updateV();
}

void FormWidgetChoice::deselectAll ()
{
  if (isReadOnly()) {
    error(-1, "FormWidgetChoice::deselectAll called on a read only field\n");
    return;
  }
  parent->deselectAll();
  _updateV();
}

GooString* FormWidgetChoice::getEditChoice ()
{
  if (!hasEdit()) {
    error(-1, "FormFieldChoice::getEditChoice called on a non-editable choice\n");
    return NULL;
  }
  return parent->getEditChoice();
}

bool FormWidgetChoice::isSelected (int i)
{
  if (!_checkRange(i)) return false;
  return parent->isSelected(i);
}

void FormWidgetChoice::setEditChoice (GooString* new_content)
{
  if (isReadOnly()) {
    error(-1, "FormWidgetText::setEditChoice called on a read only field\n");
    return;
  }
  if (!hasEdit()) {
    error(-1, "FormFieldChoice::setEditChoice : trying to edit an non-editable choice\n");
    return;
  }

  if (new_content == NULL) {
    parent->setEditChoice(NULL);
  } else {
    //append the unicode marker <FE FF> if needed
    if (!new_content->hasUnicodeMarker()) {
      new_content->insert(0, 0xff);
      new_content->insert(0, 0xfe);
    }
    parent->setEditChoice(new_content);
  }
  _updateV();
}

int FormWidgetChoice::getNumChoices() 
{ 
  return parent->getNumChoices(); 
}

GooString* FormWidgetChoice::getChoice(int i) 
{ 
  return parent->getChoice(i); 
}


bool FormWidgetChoice::isCombo () const 
{ 
  return parent->isCombo(); 
}

bool FormWidgetChoice::hasEdit () const 
{ 
  return parent->hasEdit(); 
}

bool FormWidgetChoice::isMultiSelect () const 
{ 
  return parent->isMultiSelect(); 
}

bool FormWidgetChoice::noSpellCheck () const 
{ 
  return parent->noSpellCheck(); 
}

bool FormWidgetChoice::commitOnSelChange () const 
{ 
  return parent->commitOnSelChange(); 
}

bool FormWidgetChoice::isReadOnly () const
{
  return parent->isReadOnly();
}

bool FormWidgetChoice::isListBox () const
{
  return parent->isListBox();
}

FormWidgetSignature::FormWidgetSignature(XRef *xrefA, Object *aobj, unsigned num, Ref ref, FormFieldSignature *p) : FormWidget(xrefA, aobj, num, ref)
{
  parent = p;
  type = formSignature;
}

bool FormWidgetSignature::isReadOnly () const
{
  return parent->isReadOnly();
}

//========================================================================
// FormField
//========================================================================

FormField::FormField(XRef* xrefA, Object *aobj, const Ref& aref, Form* aform, FormFieldType ty) 
{
  double t;
  xref = xrefA;
  aobj->copy(&obj);
  Dict* dict = obj.getDict();
  ref.num = ref.gen = 0;
  type = ty;
  numChildren = 0;
  children = NULL;
  terminal = false;
  widgets = NULL;
  readOnly = false;
  form = aform;
  ref = aref;

  Object obj1;
  //childs
  if (dict->lookup("Kids", &obj1)->isArray()) {
    Array *array = obj1.getArray();
    int length = array->getLength();
    // Load children
    for(int i=0; i<length; i++) { 
      Object obj2,obj3;
      Object childRef;
      array->get(i, &obj2);
      array->getNF(i, &childRef);
      //field child
      if(obj2.dictLookup("FT", &obj3)->isName()) {
        if(terminal) error(-1, "Field can't have both Widget AND Field as kids\n");

        numChildren++;
        children = (FormField**)greallocn(children, numChildren, sizeof(FormField*));

        obj3.free();
        form->createFieldFromDict (&obj2, &children[numChildren-1], xrefA, childRef.getRef());
      }
      // 1 - we will handle 'collapsed' fields (field + annot in the same dict)
      // as if the annot was in the Kids array of the field
      else if (obj2.dictLookup("Subtype",&obj3)->isName()) {
        _createWidget(&obj2, childRef.getRef());       
      }
      obj2.free();
      obj3.free();
    }
  }
  obj1.free();
  // As said in 1, if this is a 'collapsed' field, behave like if we had a
  // child annot
  if (dict->lookup("Subtype", &obj1)->isName()) {
    _createWidget(aobj, ref);
  }
  obj1.free();
 
  //flags
  if (dict->lookup("Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x2) { // 2 -> Required
      //TODO
    }
    if (flags & 0x4) { // 3 -> NoExport
      //TODO
    }
  }
  obj1.free();

}

/*FormField::FormField(FormField *dest)
{
  type = dest->type;
}*/

FormField::~FormField()
{
  if(children)
    gfree(children);
  obj.free();
}

void FormField::loadChildrenDefaults ()
{
  if(!terminal) {
    for(int i=0; i<numChildren; i++) {
      children[i]->loadChildrenDefaults();
    }
  } else {
    for (int i=0; i<numChildren; i++) {
      widgets[i]->loadDefaults();
    }
  }
}

void FormField::fillChildrenSiblingsID()
{
  if(terminal) return;
  for (int i=0; i<numChildren; i++) {
    children[i]->loadChildrenDefaults();
  }
}


void FormField::_createWidget (Object *obj, Ref aref)
{
  terminal = true;
  numChildren++;
  widgets = (FormWidget**)greallocn(widgets, numChildren, sizeof(FormWidget*));
  //ID = index in "widgets" table
  if(type==formButton) widgets[numChildren-1] = new FormWidgetButton(xref, obj, numChildren-1, aref, static_cast<FormFieldButton*>(this));
  else if (type==formText) widgets[numChildren-1] = new FormWidgetText(xref, obj, numChildren-1, aref, static_cast<FormFieldText*>(this));
  else if (type==formChoice) widgets[numChildren-1] = new FormWidgetChoice(xref, obj, numChildren-1, aref, static_cast<FormFieldChoice*>(this));
  else if (type==formSignature) widgets[numChildren-1] = new FormWidgetSignature(xref, obj, numChildren-1, aref, static_cast<FormFieldSignature*>(this));
  else  {
    error(-1, "SubType on non-terminal field, invalid document?");
    numChildren--;
    terminal = false;
  }
}

FormWidget* FormField::findWidgetByRef (Ref aref)
{
  if (terminal) {
    for(int i=0; i<numChildren; i++) {
      if (widgets[i]->getRef().num == aref.num 
          && widgets[i]->getRef().gen == aref.gen)
        return widgets[i];
    }
  } else {
    for(int i=0; i<numChildren; i++) {
      FormWidget* result = children[i]->findWidgetByRef(aref);
      if(result) return result;
    }
  }
  return NULL;
}


//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------
FormFieldButton::FormFieldButton(XRef *xrefA, Object *aobj, const Ref& ref, Form* form) 
                                 : FormField(xrefA, aobj, ref, form, formButton)
{
  type = formButton;
  Dict* dict = obj.getDict();
  active_child = -1;
  noAllOff = false;

  Object obj1;
  btype = formButtonCheck; 
  if (dict->lookup("Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x10000) { // 17 -> push button
      btype = formButtonPush;
    } else if (flags & 0x8000) { // 16 -> radio button
      btype = formButtonRadio;
      if (flags & 0x4000) { // 15 -> noToggleToOff
        noAllOff = true;
      }
    } 
    if (flags & 0x1000000) { // 26 -> radiosInUnison
      error(-1, "FormFieldButton:: radiosInUnison flag unimplemented, please report a bug with a testcase\n");
    } 
  }
  obj1.free();
}

void FormFieldButton::fillChildrenSiblingsID()
{
  for(int i=0; i<numChildren; i++) {
    FormWidgetButton *btn = static_cast<FormWidgetButton*>(widgets[i]);
    btn->setNumSiblingsID(numChildren-1);
    for(int j=0, counter=0; j<numChildren; j++) {
      if (i == j) continue;
      btn->setSiblingsID(counter, widgets[j]->getID());
      counter++;
    }
  }
}

GBool FormFieldButton::setState (int num, GBool s) 
{
  if (readOnly) {
    error(-1, "FormFieldButton::setState called on a readOnly field\n");
    return gFalse;
  }
  
  if(btype == formButtonRadio) {
    if (!s && noAllOff)
      return gFalse; //don't allow to set all radio to off

    if (s == gTrue) {
      active_child = num;
      for(int i=0; i<numChildren; i++) {
        if (i==active_child) continue;
        static_cast<FormWidgetButton*>(widgets[i])->setState(gFalse, gTrue);
      }

      //The parent field's V entry holds a name object corresponding to the ap-
      //pearance state of whichever child field is currently in the on state
      if (active_child >= 0) {
        FormWidgetButton* actChild = static_cast<FormWidgetButton*>(widgets[active_child]);
        if (actChild->getOnStr()) {
          Object *obj1 = new Object();
          obj1->initName(actChild->getOnStr());
          obj.getDict()->set("V", obj1);
          xref->setModifiedObject(&obj, ref);
        }
      }
    } else {
      active_child = -1;
      Object *obj1 = new Object();
      obj1->initName("Off");
      obj.getDict()->set("V", obj1);
      xref->setModifiedObject(&obj, ref);
    }
  }
  return gTrue;
}

FormFieldButton::~FormFieldButton()
{
}

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------
FormFieldText::FormFieldText(XRef *xrefA, Object *aobj, const Ref& ref, Form* form) 
                             : FormField(xrefA, aobj, ref, form, formText)
{
  type = formText;
  Dict* dict = obj.getDict();
  Object obj1;
  content = NULL;
  multiline = password = fileSelect = doNotSpellCheck = doNotScroll = comb = richText = false;

  if (dict->lookup("Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x1000) // 13 -> Multiline
      multiline = true;
    if (flags & 0x2000) // 14 -> Password
      password = true;
    if (flags & 0x100000) // 21 -> FileSelect
      fileSelect = true;
    if (flags & 0x400000)// 23 -> DoNotSpellCheck
      doNotSpellCheck = true;
    if (flags & 0x800000) // 24 -> DoNotScroll
      doNotScroll = true;
    if (flags & 0x1000000) // 25 -> Comb
      comb = true;
    if (flags & 0x2000000)// 26 -> RichText
      richText = true;
  }
  obj1.free();
}

GooString* FormFieldText::getContentCopy ()
{
  if (!content) return NULL;
  return new GooString(*content);
}

void FormFieldText::setContentCopy (GooString* new_content)
{
  if(content) {
    delete content; 
  }
  content = new_content->copy();
}

FormFieldText::~FormFieldText()
{

}


//------------------------------------------------------------------------
// FormFieldChoice
//------------------------------------------------------------------------
FormFieldChoice::FormFieldChoice(XRef *xrefA, Object *aobj, const Ref& ref, Form* form) 
                                 : FormField(xrefA, aobj, ref, form, formChoice)
{
  numChoices = 0;
  choices = NULL;
  editedChoice = NULL;

  Dict* dict = obj.getDict();
  Object obj1;

  combo = edit = multiselect = doNotSpellCheck = doCommitOnSelChange = false;

  if (dict->lookup("Ff", &obj1)->isInt()) {
    int flags = obj1.getInt();
    if (flags & 0x20000) // 18 -> Combo
      combo = true; 
    if (flags & 0x40000) // 19 -> Edit
      edit = true;
    if (flags & 0x200000) // 22 -> MultiSelect
      multiselect = true;
    if (flags & 0x400000) // 23 -> DoNotSpellCheck
      doNotSpellCheck = true;
    if (flags & 0x4000000) // 27 -> CommitOnSelChange
      doCommitOnSelChange = true;
  }
  obj1.free();

}

FormFieldChoice::~FormFieldChoice()
{
  for (int i=0; i<numChoices; i++) {
    delete choices[i].exportVal;
    delete choices[i].optionName;
  }
  delete [] choices;
}

void FormFieldChoice::deselectAll ()
{
  for(int i=0; i<numChoices; i++) {
    choices[i].selected = false;
  }
}

void FormFieldChoice::toggle (int i)
{
  choices[i].selected = !choices[i].selected;
}

void FormFieldChoice::select (int i)
{
  if (!multiselect) 
    deselectAll();
  choices[i].selected = true;
}

void FormFieldChoice::setEditChoice (GooString* new_content)
{
  if (editedChoice)
    delete editedChoice;

  deselectAll();

  editedChoice = new_content->copy();
}

GooString* FormFieldChoice::getEditChoice ()
{
  return editedChoice;
}

int FormFieldChoice::getNumSelected ()
{
  int cnt = 0;
  for(int i=0; i<numChoices; i++) {
    if (choices[i].selected)
      cnt++;
  }
  return cnt;
}

void FormFieldChoice::_createChoicesTab ()
{
  choices = new ChoiceOpt[numChoices]; 
  for(int i=0; i<numChoices; i++) {
    choices[i].selected = false;
  }
}

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------
FormFieldSignature::FormFieldSignature(XRef *xrefA, Object *dict, const Ref& ref, Form* form)
                                      : FormField(xrefA, dict, ref, form, formSignature)
{
}

FormFieldSignature::~FormFieldSignature()
{

}

//------------------------------------------------------------------------
// Form
//------------------------------------------------------------------------

Form::Form(XRef *xrefA, Object* acroForm)
{
  Object obj1;
  xref = xrefA;
  Array *array = acroForm->dictLookup("Fields",&obj1)->getArray();
  obj1.free();
  if(!array) {
    error(-1, "Can't get Fields array\n");
  }
  size = 0;
  numFields = 0;
  rootFields = NULL;
  for(int i=0; i<array->getLength(); i++) {
    Object oref;
    array->get(i, &obj1);
    array->getNF(i, &oref);
    if (!oref.isRef()) {
      error(-1, "Direct object in rootFields");
      continue;
    }

    if (numFields >= size) {
      size += 16;
      rootFields = (FormField**)greallocn(rootFields,size,sizeof(FormField*));
    }

    createFieldFromDict (&obj1, &rootFields[numFields++], xrefA, oref.getRef());

    //Mark readonly field
    Object obj3;
    if (obj1.dictLookup("Ff", &obj3)->isInt()) {
      int flags = obj3.getInt();
      if (flags & 0x1)
        rootFields[numFields-1]->setReadOnly(true);
    }
    obj3.free();

    obj1.free();
    oref.free();
  }

  checkForNeedAppearances(acroForm); 
}

Form::~Form() {
  int i;
  for(i = 0; i< numFields; ++i)
          delete rootFields[i];
  delete [] rootFields;
}

void Form::checkForNeedAppearances (Object *acroForm)
{
  //NeedAppearances needs to be set to 'true' in the AcroForm entry of the Catalog to enable dynamic appearance generation
  Object* catalog = new Object();
  Ref catRef;
  catRef.gen = xref->getRootGen();
  catRef.num = xref->getRootNum();
  catalog = xref->getCatalog(catalog);
  catalog->dictLookup("AcroForm", acroForm);
  Object obj1;
  obj1.initBool(true);
  acroForm->dictSet("NeedAppearances", &obj1);
  catalog->dictSet("AcroForm", acroForm);
  xref->setModifiedObject(catalog, catRef);
}


void Form::createFieldFromDict (Object* obj, FormField** ptr, XRef *xrefA, const Ref& pref)
{
    Object obj2;
    if(obj->dictLookup("FT", &obj2)->isName("Btn")) {
      (*ptr) = new FormFieldButton(xrefA, obj, pref, this);
    } else if (obj2.isName("Tx")) {
      (*ptr) = new FormFieldText(xrefA, obj, pref, this);
    } else if (obj2.isName("Ch")) {
      (*ptr) = new FormFieldChoice(xrefA, obj, pref, this);
    } else if (obj2.isName("Sig")) {
      (*ptr) = new FormFieldSignature(xrefA, obj, pref, this);
    } else { //we don't have an FT entry => non-terminal field
      (*ptr) = new FormField(xrefA, obj, pref, this);
    }
    obj2.free();
    (*ptr)->loadChildrenDefaults();
}

void Form::postWidgetsLoad ()
{
 for(int i=0; i<numFields; i++) {
   rootFields[i]->fillChildrenSiblingsID(); 
 }
}

FormWidget* Form::findWidgetByRef (Ref aref)
{
  for(int i=0; i<numFields; i++) {
    FormWidget *result = rootFields[i]->findWidgetByRef(aref);
    if(result) return result;
  }
  return NULL;
}

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

FormPageWidgets::FormPageWidgets (XRef *xrefA, Object* annots, unsigned int page, Form *form)
{
  Object obj1;
  numWidgets = 0;
  widgets = NULL;
  xref = xrefA;
  if (annots->isArray() && form) {
    size = annots->arrayGetLength();
    widgets = (FormWidget**)greallocn(widgets, size, sizeof(FormWidget*));

    /* For each entry in the page 'Annots' dict, try to find
       a matching form field */
    for (int i = 0; i < size; ++i) {
      if (!annots->arrayGetNF(i, &obj1)->isRef())  {
        /* Since all entry in a form field's kid dict needs to be
           indirect references, if this annot isn't indirect, it isn't 
           related to a form field */
        obj1.free();
        continue;
      }
      Ref r = obj1.getRef();

      /* Try to find a form field which either has this Annot in its Kids entry
          or  is merged with this Annot */
      FormWidget* tmp = form->findWidgetByRef(r);
      if(tmp) {
        // We've found a corresponding form field, link it
        tmp->setID(FormWidget::encodeID(page, numWidgets));
        widgets[numWidgets++] = tmp;
        //create a temporary Annot to get the font size
        Object obj2;
        if (annots->arrayGet(i, &obj2)->isDict()) {
          Annot* ann = new Annot(xref, NULL ,obj2.getDict(), NULL);
          tmp->setFontSize(ann->getFontSize());
          delete ann;
        }
        obj2.free();
      } 
      
      obj1.free();
    }
  } 
}

