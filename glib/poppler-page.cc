/* poppler.h: glib wrapper for poppler
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

#include <math.h>

#include <goo/GooList.h>
#include <splash/SplashBitmap.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>
#include <SplashOutputDev.h>
#include <TextOutputDev.h>

#include "poppler.h"
#include "poppler-private.h"

enum {
	PROP_0,
	PROP_LABEL
};

typedef struct _PopplerPageClass PopplerPageClass;
struct _PopplerPageClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerPage, poppler_page, G_TYPE_OBJECT);

PopplerPage *
_poppler_page_new (PopplerDocument *document, Page *page, int index)
{
  PopplerPage *poppler_page;

  poppler_page = (PopplerPage *) g_object_new (POPPLER_TYPE_PAGE, NULL);
  poppler_page->document = document;
  poppler_page->page = page;
  poppler_page->index = index;

  return poppler_page;
}

static void
poppler_page_finalize (GObject *object)
{
  /* page->page is owned by the document */
}

void
poppler_page_get_size (PopplerPage *page,
		       double      *width,
		       double      *height)
{
  if (width != NULL)
    *width = page->page->getWidth ();
  if (height != NULL)
    *height = page->page->getHeight ();
}

int
poppler_page_get_index (PopplerPage *page)
{
  return page->index;
}

/**
 * poppler_page_render_to_pixbuf:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner
 * @src_y: y coordinate of upper left corner
 * @src_width: width of rectangle to render
 * @src_height: height of rectangle to render
 * @ppp: pixels per point
 * @pixbuf: pixbuf to render into
 * @dest_x: x coordinate of offset into destination
 * @dest_y: y cooridnate of offset into destination
 * 
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.  The rectangle is
 * rendered into the specified pixmap with the upper left corner
 * placed at (dest_x, dest_y).
 **/
void
poppler_page_render_to_pixbuf (PopplerPage *page,
			       int src_x, int src_y,
			       int src_width, int src_height,
			       double scale,
			       GdkPixbuf *pixbuf,
			       int dest_x, int dest_y)
{
  SplashOutputDev *output_dev;
  SplashColor white;
  SplashBitmap *bitmap;
  SplashColorPtr color_ptr;
  int splash_width, splash_height, splash_rowstride;
  int pixbuf_rowstride, pixbuf_n_channels;
  guchar *splash_data, *pixbuf_data, *src, *dst;
  int x, y;

  white.rgb8 = splashMakeRGB8 (0xff, 0xff, 0xff);
  output_dev = new SplashOutputDev(splashModeRGB8, gFalse, white);

  output_dev->startDoc(page->document->doc->getXRef ());

  page->page->displaySlice(output_dev, 72.0 * scale, 72.0 * scale,
			   0, /* Rotate */
			   gTrue, /* Crop */
			   src_x, src_y,
			   src_width, src_height,
			   NULL, /* links */
			   page->document->doc->getCatalog ());

  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();

  splash_width = bitmap->getWidth ();
  splash_height = bitmap->getHeight ();
  splash_data = (guchar *) color_ptr.rgb8;
  splash_rowstride = bitmap->getRowSize ();

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  /* FIXME: Clip the rectangle to pixbuf. */

  for (y = 0; y < splash_height; y++) {
    src = splash_data + y * splash_rowstride;
    dst = pixbuf_data + (dest_y + y) * pixbuf_rowstride +
      dest_x * pixbuf_n_channels;
    for (x = 0; x < splash_width; x++) {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst += pixbuf_n_channels;
      src += 4;      
    }
  }

  delete output_dev;
}

static void 
destroy_thumb_data (guchar *pixels, gpointer data)
{
  gfree (pixels);
}

GdkPixbuf *
poppler_page_get_thumbnail (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;

  if (!page->page->loadThumb (&data, &width, &height, &rowstride))
    return NULL;

  return gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB,
				   FALSE, 8, width, height, rowstride,
				   destroy_thumb_data, NULL);
}


/**
 * poppler_page_get_thumbnail_size:
 * @page: A #PopplerPage
 * @width: return location for width
 * @height: return location for height
 * 
 * Returns %TRUE if @page has a thumbnail associated with it.  It also fills in
 * @width and @height with the width and height of the thumbnail.  The values of
 * width and height are not changed if no appropriate thumbnail exists.
 * 
 * Return value: %TRUE, if @page has a thumbnail associated with it.
 **/
gboolean
poppler_page_get_thumbnail_size (PopplerPage *page,
				 int         *width,
				 int         *height)
{
  Object thumb;
  Dict *dict;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (width != NULL, FALSE);
  g_return_val_if_fail (height != NULL, FALSE);

  page->page->getThumb (&thumb);
  if (thumb.isNull ())
    {
      thumb.free ();
      return FALSE;
    }

  dict = thumb.streamGetDict();

  /* Theoretically, this could succeed and you would still fail when loading the
   * thumb */
  if (dict->lookupInt ("Width", "W", width)  &&
      dict->lookupInt ("Height", "H", height))
    retval = TRUE;

  thumb.free ();

  return retval;
}

static void
poppler_page_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
  PopplerPage *page = POPPLER_PAGE (object);
  GooString label;

  switch (prop_id)
    {
    case PROP_LABEL:
      page->document->doc->getCatalog ()->indexToLabel (page->index, &label);
      g_value_set_string (value, label.getCString());
      break;
    }
}

static void
poppler_page_class_init (PopplerPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->finalize = poppler_page_finalize;
  gobject_class->get_property = poppler_page_get_property;

  pspec = g_param_spec_string ("label",
			       "Page Label",
			       "The label of the page",
			       NULL,
			       G_PARAM_READABLE);
  g_object_class_install_property (G_OBJECT_CLASS (klass),
				   PROP_LABEL,
				   pspec);
}

static void
poppler_page_init (PopplerPage *page)
{
}
