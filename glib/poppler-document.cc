/* poppler-document.cc: glib wrapper for poppler
 * Copyright (C) 2005, Red Hat, Inc.
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

#include <goo/GooList.h>
#include <splash/SplashBitmap.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>
#include <SplashOutputDev.h>
#include <Stream.h>

#include "poppler.h"
#include "poppler-private.h"

enum {
	PROP_0,
	PROP_TITLE
};

typedef struct _PopplerDocumentClass PopplerDocumentClass;
struct _PopplerDocumentClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerDocument, poppler_document, G_TYPE_OBJECT);

PopplerDocument *
poppler_document_new_from_file (const char  *uri,
				const char  *password,
				GError     **error)
{
  PopplerDocument *document;
  PDFDoc *newDoc;
  GooString *filename_g;
  GooString *password_g;
  int err;
  char *filename;

  document = (PopplerDocument *) g_object_new (POPPLER_TYPE_DOCUMENT, NULL);
  
  if (!globalParams) {
    globalParams = new GlobalParams("/etc/xpdfrc");
    globalParams->setupBaseFontsFc(NULL);
  }

  filename = g_filename_from_uri (uri, NULL, error);
  if (!filename)
    return NULL;

  filename_g = new GooString (filename);
  g_free (filename);

  password_g = NULL;
  if (password != NULL)
    password_g = new GooString (password);

  newDoc = new PDFDoc(filename_g, password_g, password_g);
  if (password_g)
    delete password_g;

  if (!newDoc->isOk()) {
    err = newDoc->getErrorCode();
    delete newDoc;
    if (err == errEncrypted) {
      g_set_error (error, POPPLER_ERROR,
		   POPPLER_ERROR_ENCRYPTED,
		   "Document is encrypted.");
    } else {
      g_set_error (error, G_FILE_ERROR,
		   G_FILE_ERROR_FAILED,
		   "Failed to load document (error %d) '%s'\n",
		   err,
		   uri);
    }

    return NULL;
  }

  document->doc = newDoc;

  return document;
}

gboolean
poppler_document_save (PopplerDocument  *document,
		       const char       *uri,
		       GError          **error)
{
  char *filename;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), FALSE);

  filename = g_filename_from_uri (uri, NULL, error);
  if (filename != NULL) {
    GooString *fname = new GooString (filename);

    retval = document->doc->saveAs (fname);
  }

  return retval;
}

static void
poppler_document_finalize (GObject *object)
{
  PopplerDocument *document = POPPLER_DOCUMENT (object);

  delete document->doc;
}

static gboolean
popper_document_save (PopplerDocument  *document,
		      const char       *uri,
		      GError          **error)
{
  char *filename;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), FALSE);

  filename = g_filename_from_uri (uri, NULL, error);
  if (filename != NULL) {
    GooString *filename_g = new GooString (filename);

    retval = document->doc->saveAs (filename_g);
  }

  return retval;
}

int
poppler_document_get_n_pages (PopplerDocument *document)
{
  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), 0);

  return document->doc->getNumPages();
}

PopplerPage *
poppler_document_get_page (PopplerDocument  *document,
			   int               index)
{
  Catalog *catalog;
  Page *page;

  g_return_val_if_fail (0 <= index &&
			index < poppler_document_get_n_pages (document),
			NULL);

  catalog = document->doc->getCatalog();
  page = catalog->getPage (index + 1);

  return _poppler_page_new (document, page, index);
}

PopplerPage *
poppler_document_get_page_by_label (PopplerDocument  *document,
				    const char       *label)
{
  Catalog *catalog;
  GooString label_g(label);
  int index;

  catalog = document->doc->getCatalog();
  if (!catalog->labelToIndex (&label_g, &index))
    return NULL;

  return poppler_document_get_page (document, index);
}

static gboolean
has_unicode_marker (GooString *string)
{
  return ((string->getChar (0) & 0xff) == 0xfe &&
	  (string->getChar (1) & 0xff) == 0xff);
}

static void
info_dict_get_string (Dict *info_dict, const gchar *key, GValue *value)
{
  Object obj;
  GooString *goo_value;
  gchar *result;

  if (!info_dict->lookup ((gchar *)key, &obj)->isString ()) {
    obj.free ();
    return;
  }

  goo_value = obj.getString ();

  if (has_unicode_marker (goo_value)) {
    result = g_convert (goo_value->getCString () + 2,
			goo_value->getLength () - 2,
			"UTF-8", "UTF-16BE", NULL, NULL, NULL);
  } else {
    result = g_strndup (goo_value->getCString (), goo_value->getLength ());
  }

  obj.free ();

  g_value_set_string (value, result);

  g_free (result);
}

static void
poppler_document_get_property (GObject *object,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *pspec)
{
  PopplerDocument *document = POPPLER_DOCUMENT (object);
  Object info;

  document->doc->getDocInfo (&info);
  if (!info.isDict ())
    return;

  switch (prop_id)
    {
    case PROP_TITLE:
      info_dict_get_string (info.getDict(), "Title", value);
      break;
    }
}

static void
poppler_document_class_init (PopplerDocumentClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->finalize = poppler_document_finalize;
  gobject_class->get_property = poppler_document_get_property;

  pspec = g_param_spec_string ("title",
			       "Document Title",
			       "The title of the document",
			       NULL,
			       G_PARAM_READABLE);
  g_object_class_install_property (G_OBJECT_CLASS (klass),
				   PROP_TITLE,
				   pspec);
}

static void
poppler_document_init (PopplerDocument *document)
{
}


/* PopplerIndexIter: For determining the index of a tree */
struct _PopplerIndexIter
{
	PopplerDocument *document;
	GooList *items;
	int index;
};

PopplerIndexIter *
poppler_index_iter_new (PopplerDocument *document)
{
	PopplerIndexIter *iter;
	Outline *outline;
	GooList *items;

	outline = document->doc->getOutline();
	if (outline == NULL)
		return NULL;

	items = outline->getItems();
	if (items == NULL)
		return NULL;

	iter = g_new0 (PopplerIndexIter, 1);
	iter->document = (PopplerDocument *) g_object_ref (document);
	iter->items = items;
	iter->index = 0;

	return iter;
}

PopplerIndexIter *
poppler_index_iter_get_child (PopplerIndexIter *parent)
{
	PopplerIndexIter *child;
	OutlineItem *item;

	g_return_val_if_fail (parent != NULL, NULL);
	
	item = (OutlineItem *)parent->items->get (parent->index);
	item->open ();
	if (! (item->hasKids() && item->getKids()) )
		return NULL;

	child = g_new0 (PopplerIndexIter, 1);
	child->document = (PopplerDocument *)g_object_ref (parent->document);
	child->items = item->getKids ();

	g_assert (child->items);

	return child;
}



static gchar *
unicode_to_char (Unicode *unicode,
		 int      len)
{
	static UnicodeMap *uMap = NULL;
	if (uMap == NULL) {
		GooString *enc = new GooString("UTF-8");
		uMap = globalParams->getUnicodeMap(enc);
		uMap->incRefCnt ();
		delete enc;
	}
		
	GooString gstr;
	gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;

	for (i = 0; i < len; ++i) {
		n = uMap->mapUnicode(unicode[i], buf, sizeof(buf));
		gstr.append(buf, n);
	}

	return g_strdup (gstr.getCString ());
}



void
poppler_index_iter_get_values (PopplerIndexIter  *iter,
			       char             **text,
			       char             **link_string,
			       int               *page)
{
	OutlineItem *item;
	LinkAction *action;

	g_return_if_fail (iter != NULL);
	g_return_if_fail (text != NULL);
	g_return_if_fail (link_string != NULL);
	g_return_if_fail (page != NULL);

	item = (OutlineItem *)iter->items->get (iter->index);
	*text = unicode_to_char (item->getTitle(),
				 item->getTitleLength ());

	*page = -1;
	action = item->getAction ();
	if (action->getKind () == actionGoTo) {
		LinkDest *link_dest;
		LinkGoTo *link_goto;
		Ref page_ref;
		gint page_num = -1;
		GooString *named_dest;

		link_goto = dynamic_cast <LinkGoTo *> (action);
		link_dest = link_goto->getDest ();
		named_dest = link_goto->getNamedDest ();

		if (link_dest != NULL) {
			link_dest = link_dest->copy ();
		} else if (named_dest != NULL) {
			named_dest = named_dest->copy ();
			link_dest = iter->document->doc->findDest (named_dest);
			delete named_dest;
		}
		if (link_dest != NULL) {
			if (link_dest->isPageRef ()) {
				page_ref = link_dest->getPageRef ();
				page_num = iter->document->doc->findPage (page_ref.num, page_ref.gen);
			} else {
				page_num = link_dest->getPageNum ();
			}
			delete link_dest;
		}
		if (page_num > 0)
			*page = page_num - 1;
	}
}

gboolean
poppler_index_iter_next (PopplerIndexIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	iter->index++;
	if (iter->index >= iter->items->getLength())
		return FALSE;

	return TRUE;
}

void
poppler_index_iter_free (PopplerIndexIter *iter)
{
	if (iter == NULL)
		return;

	g_object_unref (iter->document);
//	delete iter->items;
	g_free (iter);
	
}
