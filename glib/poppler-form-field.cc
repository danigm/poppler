/* poppler-form-field.cc: glib interface to poppler
 *
 * Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2006 Julien Rebetez
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

#include "poppler.h"
#include "poppler-private.h"

typedef struct _PopplerFormFieldClass PopplerFormFieldClass;
struct _PopplerFormFieldClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerFormField, poppler_form_field, G_TYPE_OBJECT);

static void
poppler_form_field_finalize (GObject *object)
{
  PopplerFormField *field = POPPLER_FORM_FIELD (object);

  if (field->document)
    {
      g_object_unref (field->document);
      field->document = NULL;
    }
  field->widget = NULL;

  G_OBJECT_CLASS (poppler_form_field_parent_class)->finalize (object);
}

static void
poppler_form_field_init (PopplerFormField *field)
{
}

static void
poppler_form_field_class_init (PopplerFormFieldClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_form_field_finalize;
}

PopplerFormField *
_poppler_form_field_new (PopplerDocument *document,
			 FormWidget      *field)
{
  PopplerFormField *poppler_field;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);
  g_return_val_if_fail (field != NULL, NULL);

  poppler_field = POPPLER_FORM_FIELD (g_object_new (POPPLER_TYPE_FORM_FIELD, NULL));

  poppler_field->document = (PopplerDocument *)g_object_ref (document);
  poppler_field->widget = field;
  
  return poppler_field;
}

/* Public methods */
PopplerFormFieldType
poppler_form_field_get_field_type (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), POPPLER_FORM_FIELD_UNKNOWN);
  
  switch (field->widget->getType ())
  {
    case formButton:
      return POPPLER_FORM_FIELD_BUTTON;
    case formText:
      return POPPLER_FORM_FIELD_TEXT;
    case formChoice:
      return POPPLER_FORM_FIELD_CHOICE;
    default:
      g_warning ("Unsupported Form Field Type");
  }

  return POPPLER_FORM_FIELD_UNKNOWN;
}

gint
poppler_form_field_get_id (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), -1);
  
  return field->widget->getID ();
}

gdouble
poppler_form_field_get_font_size (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), 0);
  
  return field->widget->getFontSize ();
}

gboolean
poppler_form_field_is_read_only (PopplerFormField *field)
{
  g_return_val_if_fail (POPPLER_IS_FORM_FIELD (field), FALSE);

  return field->widget->isReadOnly ();
}

/* Button Field */
gboolean
poppler_form_field_button_get_state (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formButton, FALSE);
  
  return static_cast<FormWidgetButton*>(field->widget)->getState ();
}

void
poppler_form_field_button_set_state (PopplerFormField *field,
				     gboolean          state)
{
  g_return_if_fail (field->widget->getType () == formButton);

  static_cast<FormWidgetButton*>(field->widget)->setState ((GBool)state);
}

/* Text Field */
PopplerFormTextType
poppler_form_field_text_get_text_type (PopplerFormField *field)
{
  FormWidgetText *text_field;
  
  g_return_val_if_fail (field->widget->getType () == formText, POPPLER_FORM_TEXT_NORMAL);

  text_field = static_cast<FormWidgetText*>(field->widget);
  
  if (text_field->isMultiline ())
    return POPPLER_FORM_TEXT_MULTILINE;
  else if (text_field->isPassword ())
    return POPPLER_FORM_TEXT_PASSWORD;
  else if (text_field->isFileSelect ())
    return POPPLER_FORM_TEXT_FILE_SELECT;

  return POPPLER_FORM_TEXT_NORMAL;
}

gchar *
poppler_form_field_text_get_text (PopplerFormField *field)
{
  FormWidgetText *text_field;
  GooString      *tmp;

  g_return_val_if_fail (field->widget->getType () == formText, NULL);

  text_field = static_cast<FormWidgetText*>(field->widget);
  tmp = text_field->getContent ();

  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

void
poppler_form_field_text_set_text (PopplerFormField *field,
				  const gchar      *text)
{
  GooString *tmp;
	
  g_return_if_fail (field->widget->getType () == formText);

  tmp = new GooString (text, g_utf8_strlen (text, -1));
  static_cast<FormWidgetText*>(field->widget)->setContent (tmp);
  delete tmp;
}

gboolean
poppler_form_field_text_do_spell_check (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return !static_cast<FormWidgetText*>(field->widget)->noSpellCheck ();
}

gboolean
poppler_form_field_text_do_scroll (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return !static_cast<FormWidgetText*>(field->widget)->noScroll ();
}

gboolean
poppler_form_field_text_is_rich_text (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formText, FALSE);

  return static_cast<FormWidgetText*>(field->widget)->isRichText ();
}

/* Choice Field */
PopplerFormChoiceType
poppler_form_field_choice_get_choice_type (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, POPPLER_FORM_CHOICE_COMBO);

  if (static_cast<FormWidgetChoice*>(field->widget)->isCombo ())
    return POPPLER_FORM_CHOICE_COMBO;
  else
    return POPPLER_FORM_CHOICE_LIST;
}

gboolean
poppler_form_field_choice_is_editable (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->hasEdit ();
}

gboolean
poppler_form_field_choice_can_select_multiple (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->isMultiSelect ();
}

gboolean
poppler_form_field_choice_do_spell_check (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return !static_cast<FormWidgetChoice*>(field->widget)->noSpellCheck ();
}

gint
poppler_form_field_choice_get_n_items (PopplerFormField *field)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, -1);

  return static_cast<FormWidgetChoice*>(field->widget)->getNumChoices ();
}

gchar *
poppler_form_field_choice_get_item (PopplerFormField *field,
				    gint              index)
{
  GooString *tmp;
  
  g_return_val_if_fail (field->widget->getType () == formChoice, NULL);

  tmp = static_cast<FormWidgetChoice*>(field->widget)->getChoice (index);
  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}

gboolean
poppler_form_field_choice_is_item_selected (PopplerFormField *field,
					    gint              index)
{
  g_return_val_if_fail (field->widget->getType () == formChoice, FALSE);

  return static_cast<FormWidgetChoice*>(field->widget)->isSelected (index);
}

void
poppler_form_field_choice_select_item (PopplerFormField *field,
				       gint              index)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->select (index);
}

void
poppler_form_field_choice_unselect_all (PopplerFormField *field)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->deselectAll ();
}

void
poppler_form_field_choice_toggle_item (PopplerFormField *field,
				       gint              index)
{
  g_return_if_fail (field->widget->getType () == formChoice);

  static_cast<FormWidgetChoice*>(field->widget)->toggle (index);
}

void
poppler_form_field_choice_set_text (PopplerFormField *field,
				    const gchar      *text)
{
  GooString *tmp;
  
  g_return_if_fail (field->widget->getType () == formChoice);
  
  tmp = new GooString (text, g_utf8_strlen (text, -1));
  static_cast<FormWidgetChoice*>(field->widget)->setEditChoice (tmp);
  delete tmp;
}

gchar *
poppler_form_field_choice_get_text (PopplerFormField *field)
{
  GooString *tmp;
  
  g_return_val_if_fail (field->widget->getType () == formChoice, NULL);

  tmp = static_cast<FormWidgetChoice*>(field->widget)->getEditChoice ();
  return tmp ? _poppler_goo_string_to_utf8 (tmp) : NULL;
}
