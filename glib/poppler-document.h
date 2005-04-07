/* poppler-document.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
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

#ifndef __POPPLER_DOCUMENT_H__
#define __POPPLER_DOCUMENT_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "poppler.h"
#include "poppler-action.h"

G_BEGIN_DECLS

#define POPPLER_TYPE_DOCUMENT             (poppler_document_get_type ())
#define POPPLER_DOCUMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_DOCUMENT, PopplerDocument))
#define POPPLER_IS_DOCUMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_DOCUMENT))

typedef struct _PopplerDocument PopplerDocument;
typedef struct _PopplerIndexIter PopplerIndexIter;
typedef struct _PopplerPage PopplerPage;
typedef struct _PopplerPSFile PopplerPSFile;

PopplerDocument *poppler_document_new_from_file     (const char       *uri,
						     const char       *password,
						     GError          **error);
gboolean         poppler_document_save              (PopplerDocument  *document,
						     const char       *uri,
						     GError          **error);
int              poppler_document_get_n_pages       (PopplerDocument  *document);
PopplerPage     *poppler_document_get_page          (PopplerDocument  *document,
						     int               page);
PopplerPage     *poppler_document_get_page_by_label (PopplerDocument  *document,
						     const char       *label);

GType            poppler_document_get_type          (void) G_GNUC_CONST;

/* Interface for getting the Index of a poppler_document */
PopplerIndexIter *poppler_index_iter_new        (PopplerDocument   *document);
PopplerIndexIter *poppler_index_iter_get_child  (PopplerIndexIter  *parent);
PopplerAction    *poppler_index_iter_get_action (PopplerIndexIter  *iter);
gboolean          poppler_index_iter_next       (PopplerIndexIter  *iter);
void              poppler_index_iter_free       (PopplerIndexIter  *iter);

/* Export to ps */
PopplerPSFile *poppler_ps_file_new   (PopplerDocument *document,
				      const char      *filename,
				      int              n_pages);
void           poppler_ps_file_free  (PopplerPSFile   *ps_file);

G_END_DECLS

#endif /* __POPPLER_DOCUMENT_H__ */
