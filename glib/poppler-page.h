/* poppler-page.h: glib interface to poppler
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

#ifndef __POPPLER_PAGE_H__
#define __POPPLER_PAGE_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "poppler.h"

G_BEGIN_DECLS

#define POPPLER_TYPE_PAGE             (poppler_page_get_type ())
#define POPPLER_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_PAGE, PopplerPage))
#define POPPLER_IS_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_PAGE))

GType      poppler_page_get_type         (void) G_GNUC_CONST;
void       poppler_page_render_to_pixbuf (PopplerPage  *page,
					  int           src_x,
					  int           src_y,
					  int           src_width,
					  int           src_height,
					  double        scale,
					  GdkPixbuf    *pixbuf,
					  int           dest_x,
					  int           dest_y);
void       poppler_page_get_size         (PopplerPage  *page,
					  double       *width,
					  double       *height);
int        poppler_page_get_index        (PopplerPage  *page);
GdkPixbuf *poppler_page_get_thumbnail    (PopplerPage  *page);
gboolean   poppler_page_get_thumbnail_size (PopplerPage  *page,
					    int       *width,
					    int       *height);

G_END_DECLS

#endif /* __POPPLER_GLIB_H__ */
