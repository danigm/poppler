/* poppler-form.h: qt4 interface to poppler
 * Copyright (C) 2007, Pino Toscano <pino@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>
#include <QtCore/QSizeF>
#include <Form.h>
#include <Object.h>

#include "poppler-form.h"
#include "poppler-private.h"
#include "poppler-annotation-helper.h"

#include <math.h>

namespace Poppler {

FormField::FormField(DocumentData *doc, ::Page *p, ::FormWidget *w)
  : m_formData(new FormFieldData(doc, p, w))
{
  // reading the coords
  double left, top, right, bottom;
  w->getRect(&left, &bottom, &right, &top);
  // build a normalized transform matrix for this page at 100% scale
  GfxState gfxState( 72.0, 72.0, p->getMediaBox(), p->getRotate(), gTrue );
  double * gfxCTM = gfxState.getCTM();
  double MTX[6];
  for ( int i = 0; i < 6; i+=2 )
  {
    MTX[i] = gfxCTM[i] / p->getCropWidth();
    MTX[i+1] = gfxCTM[i+1] / p->getCropHeight();
  }
  QPointF topLeft;
  XPDFReader::transform( MTX, qMin( left, right ), qMax( top, bottom ), topLeft );
  QPointF bottomRight;
  XPDFReader::transform( MTX, qMax( left, right ), qMin( top, bottom ), bottomRight );
  m_formData->box = QRectF(topLeft, QSizeF(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y()));

  Object *obj = m_formData->fm->getObj();
  Object tmp;

  // reading the flags
  if (obj->isDict() && obj->dictLookup("Ff", &tmp)->isInt())
  {
    m_formData->flags = tmp.getInt();
  }
  tmp.free();
  // reading the widget annotation flags
  if (obj->isDict() && obj->dictLookup("F", &tmp)->isInt())
  {
    m_formData->annoflags = tmp.getInt();
  }
  tmp.free();
}

FormField::~FormField()
{
  delete m_formData;
  m_formData = 0;
}

QRectF FormField::rect() const
{
  return m_formData->box;
}

int FormField::id() const
{
  return m_formData->fm->getID();
}

QString FormField::name() const
{
  Object tmp;
  Object *obj = m_formData->fm->getObj();
  QString name;
  if (obj->dictLookup("T", &tmp)->isString())
  {
    GooString *goo = tmp.getString();
    if (goo)
      name = goo->getCString();
  }
  tmp.free();
  return name;
}

QString FormField::uiName() const
{
  Object tmp;
  Object *obj = m_formData->fm->getObj();
  QString name;
  if (obj->dictLookup("TU", &tmp)->isString())
  {
    GooString *goo = tmp.getString();
    if (goo)
      name = goo->getCString();
  }
  tmp.free();
  return name;
}

bool FormField::isReadOnly() const
{
  return m_formData->fm->isReadOnly();
}

bool FormField::isVisible() const
{
  return !(m_formData->annoflags & (1 << 1));
}


FormFieldText::FormFieldText(DocumentData *doc, ::Page *p, ::FormWidgetText *w)
  : FormField(doc, p, w)
{
}

FormFieldText::~FormFieldText()
{
}

FormField::FormType FormFieldText::type() const
{
  return FormField::FormText;
}

FormFieldText::TextType FormFieldText::textType() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  if (fwt->isFileSelect())
    return FormFieldText::FileSelect;
  else if (fwt->isMultiline())
    return FormFieldText::Multiline;
  return FormFieldText::Normal;
}

QString FormFieldText::text() const
{
  GooString *goo = static_cast<FormWidgetText*>(m_formData->fm)->getContent();
  return UnicodeParsedString(goo);
}

void FormFieldText::setText( const QString& text )
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  GooString * goo = QStringToGooString( text );
  fwt->setContent( goo );
}

bool FormFieldText::isPassword() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return fwt->isPassword();
}

bool FormFieldText::isRichText() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return fwt->isRichText();
}

int FormFieldText::maximumLength() const
{
  Object *obj = m_formData->fm->getObj();
  int maxlen = -1;
  if (!obj->isDict()) return maxlen;
  Object tmp;
  if (obj->dictLookup("MaxLen", &tmp)->isInt())
  {
    maxlen = tmp.getInt();
  }
  tmp.free();
  return maxlen;
}

Qt::Alignment FormFieldText::textAlignment() const
{
  return m_formData->textAlignment(m_formData->fm->getObj());
}

bool FormFieldText::canBeSpellChecked() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return !fwt->noSpellCheck();
}


FormFieldChoice::FormFieldChoice(DocumentData *doc, ::Page *p, ::FormWidgetChoice *w)
  : FormField(doc, p, w)
{
}

FormFieldChoice::~FormFieldChoice()
{
}

FormFieldChoice::FormType FormFieldChoice::type() const
{
  return FormField::FormChoice;
}

FormFieldChoice::ChoiceType FormFieldChoice::choiceType() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  if (fwc->isCombo())
    return FormFieldChoice::ComboBox;
  return FormFieldChoice::ListBox;
}

QStringList FormFieldChoice::choices() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  QStringList ret;
  int num = fwc->getNumChoices();
  for (int i = 0; i < num; ++i)
  {
    ret.append(UnicodeParsedString(fwc->getChoice(i)));
  }
  return ret;
}

bool FormFieldChoice::isEditable() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return fwc->isCombo() ? fwc->hasEdit() : false;
}

bool FormFieldChoice::multiSelect() const
{
//  return m_formData->flags & (1 << 21);
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return !fwc->isCombo() ? fwc->isMultiSelect() : false;
}

QList<int> FormFieldChoice::currentChoices() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  int num = fwc->getNumChoices();
  QList<int> choices;
  for ( int i = 0; i < num; ++i )
    if ( fwc->isSelected( i ) )
      choices.append( i );
  return choices;
}

void FormFieldChoice::setCurrentChoices( const QList<int> &choice )
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  fwc->deselectAll();
  for ( int i = 0; i < choice.count(); ++i )
    fwc->select( choice.at( i ) );
}

Qt::Alignment FormFieldChoice::textAlignment() const
{
  return m_formData->textAlignment(m_formData->fm->getObj());
}

bool FormFieldChoice::canBeSpellChecked() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return !fwc->noSpellCheck();
}

}
