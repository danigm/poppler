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

#include "poppler.h"
#include "poppler-private.h"

#define POPPLER_DOCUMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), POPPLER_TYPE_DOCUMENT, PopplerDocumentClass))
#define POPPLER_IS_DOCUMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), POPPLER_TYPE_DOCUMENT))
#define POPPLER_DOCUMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), POPPLER_TYPE_DOCUMENT, PopplerDocumentClass))

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
  GooString *enc;
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

  return _poppler_page_new (document, page);
}

static void
poppler_document_class_init (PopplerDocumentClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_document_finalize;
}

static void
poppler_document_init (PopplerDocument *document)
{
}
