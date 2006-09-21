/* poppler-page.cc: glib wrapper for poppler
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

#include "config.h"
#include <math.h>

#include <goo/GooList.h>
#include <splash/SplashBitmap.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>

#include "poppler.h"
#include "poppler-private.h"

enum
{
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

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

  poppler_page = (PopplerPage *) g_object_new (POPPLER_TYPE_PAGE, NULL, NULL);
  poppler_page->document = (PopplerDocument *) g_object_ref (document);
  poppler_page->page = page;
  poppler_page->index = index;

  return poppler_page;
}

static void
poppler_page_finalize (GObject *object)
{
  PopplerPage *page = POPPLER_PAGE (object);

  g_object_unref (page->document);
  page->document = NULL;

  if (page->gfx != NULL)
    delete page->gfx;
  if (page->text_dev != NULL)
    delete page->text_dev;

  /* page->page is owned by the document */
}

/**
 * poppler_page_get_size:
 * @page: A #PopplerPage
 * @width: return location for the width of @page
 * @height: return location for the height of @page
 * 
 * Gets the size of @page at the current scale and rotation.
 **/
void
poppler_page_get_size (PopplerPage *page,
		       double      *width,
		       double      *height)
{
  double page_width, page_height;
  int rotate;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  rotate = page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    page_height = page->page->getCropWidth ();
    page_width = page->page->getCropHeight ();
  } else {
    page_width = page->page->getCropWidth ();
    page_height = page->page->getCropHeight ();
  }

  if (width != NULL)
    *width = page_width;
  if (height != NULL)
    *height = page_height;
}

/**
 * poppler_page_get_index:
 * @page: a #PopplerPage
 * 
 * Returns the index of @page
 * 
 * Return value: index value of @page
 **/
int
poppler_page_get_index (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), 0);

  return page->index;
}

#if defined (HAVE_CAIRO)

typedef struct {
  unsigned char *cairo_data;
  cairo_surface_t *surface;
  cairo_t *cairo;
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 int rotation,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  CairoOutputDev *output_dev;
  cairo_surface_t *surface;
  double width, height;
  int cairo_width, cairo_height, cairo_rowstride, rotate;
  unsigned char *cairo_data;

  rotate = rotation + page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    height = page->page->getCropWidth ();
    width = page->page->getCropHeight ();
  } else {
    width = page->page->getCropWidth ();
    height = page->page->getCropHeight ();
  }

  cairo_width = (int) ceil(width * scale);
  cairo_height = (int) ceil(height * scale);

  output_dev = page->document->output_dev;
  cairo_rowstride = cairo_width * 4;
  cairo_data = (guchar *) gmalloc (cairo_height * cairo_rowstride);
  if (transparent)
      memset (cairo_data, 0x00, cairo_height * cairo_rowstride);
  else
      memset (cairo_data, 0xff, cairo_height * cairo_rowstride);

  surface = cairo_image_surface_create_for_data(cairo_data,
						CAIRO_FORMAT_ARGB32,
	  					cairo_width, cairo_height, 
						cairo_rowstride);

  output_dev_data->cairo_data = cairo_data;
  output_dev_data->surface = surface;
  output_dev_data->cairo = cairo_create (surface);
  output_dev->setCairo (output_dev_data->cairo);
}

static void
poppler_page_copy_to_pixbuf (PopplerPage *page,
			     GdkPixbuf *pixbuf,
			     OutputDevData *output_dev_data)
{
  int cairo_width, cairo_height, cairo_rowstride;
  unsigned char *pixbuf_data, *dst, *cairo_data;
  int pixbuf_rowstride, pixbuf_n_channels;
  unsigned int *src;
  int x, y;

  cairo_width = cairo_image_surface_get_width (output_dev_data->surface);
  cairo_height = cairo_image_surface_get_height (output_dev_data->surface);
  cairo_rowstride = cairo_width * 4;
  cairo_data = output_dev_data->cairo_data;

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (cairo_width > gdk_pixbuf_get_width (pixbuf))
    cairo_width = gdk_pixbuf_get_width (pixbuf);
  if (cairo_height > gdk_pixbuf_get_height (pixbuf))
    cairo_height = gdk_pixbuf_get_height (pixbuf);
  for (y = 0; y < cairo_height; y++)
    {
      src = (unsigned int *) (cairo_data + y * cairo_rowstride);
      dst = pixbuf_data + y * pixbuf_rowstride;
      for (x = 0; x < cairo_width; x++) 
	{
	  dst[0] = (*src >> 16) & 0xff;
	  dst[1] = (*src >> 8) & 0xff; 
	  dst[2] = (*src >> 0) & 0xff;
	  if (pixbuf_n_channels == 4)
	      dst[3] = (*src >> 24) & 0xff;
	  dst += pixbuf_n_channels;
	  src++;
	}
    }

  page->document->output_dev->setCairo (NULL);
  cairo_surface_destroy (output_dev_data->surface);
  cairo_destroy (output_dev_data->cairo);
  gfree (output_dev_data->cairo_data);
}

#elif defined (HAVE_SPLASH)
 
typedef struct {
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 int rotation,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  /* pft */
}

static void
poppler_page_copy_to_pixbuf(PopplerPage *page,
			    GdkPixbuf *pixbuf,
			    OutputDevData *data)
{
  SplashOutputDev *output_dev;
  SplashBitmap *bitmap;
  SplashColorPtr color_ptr;
  int splash_width, splash_height, splash_rowstride;
  int pixbuf_rowstride, pixbuf_n_channels;
  guchar *pixbuf_data, *dst;
  int x, y;

  output_dev = page->document->output_dev;

  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();

  splash_width = bitmap->getWidth ();
  splash_height = bitmap->getHeight ();
  splash_rowstride = bitmap->getRowSize ();

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (splash_width > gdk_pixbuf_get_width (pixbuf))
    splash_width = gdk_pixbuf_get_width (pixbuf);
  if (splash_height > gdk_pixbuf_get_height (pixbuf))
    splash_height = gdk_pixbuf_get_height (pixbuf);

  SplashColorPtr pixel = new Guchar[4];
  for (y = 0; y < splash_height; y++)
  {
    dst = pixbuf_data + y * pixbuf_rowstride;
    for (x = 0; x < splash_width; x++)
    {
      output_dev->getBitmap()->getPixel(x, y, pixel);
      dst[0] = pixel[0];
      dst[1] = pixel[1];
      dst[2] = pixel[2];
      if (pixbuf_n_channels == 4)
        dst[3] = 0xff;
      dst += pixbuf_n_channels;
    }
  }
  delete [] pixel;
}

#endif

#if defined (HAVE_CAIRO)

/**
 * poppler_page_render:
 * @page: the page to render from
 * @cairo: cairo context to render to
 *
 * Render the page to the given cairo context.
 **/
void
poppler_page_render (PopplerPage *page,
		     cairo_t *cairo)
{
  CairoOutputDev *output_dev;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  output_dev = page->document->output_dev;
  output_dev->setCairo (cairo);

  page->page->displaySlice(output_dev,
			   72.0, 72.0, 0,
			   gFalse, /* useMediaBox */
			   gTrue, /* Crop */
			   0, 0,
			   (int) ceil (page->page->getCropWidth ()),
			   (int) ceil (page->page->getCropHeight ()),
			   NULL, /* links */
			   page->document->doc->getCatalog ());

  output_dev->setCairo (NULL);
}

#endif

/**
 * poppler_page_render:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner  
 * @src_y: y coordinate of upper left corner  
 * @src_width: width of rectangle to render  
 * @src_height: height of rectangle to render
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render into
 *
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.
 **/
void
poppler_page_render_to_pixbuf (PopplerPage *page,
			       int src_x, int src_y,
			       int src_width, int src_height,
			       double scale,
			       int rotation,
			       GdkPixbuf *pixbuf)
{
  OutputDevData data;

  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  poppler_page_prepare_output_dev (page, scale, rotation, FALSE, &data);

  page->page->displaySlice(page->document->output_dev,
			   72.0 * scale, 72.0 * scale,
			   rotation,
			   gFalse, /* useMediaBox */
			   gTrue, /* Crop */
			   src_x, src_y,
			   src_width, src_height,
			   NULL, /* links */
			   page->document->doc->getCatalog ());

  poppler_page_copy_to_pixbuf (page, pixbuf, &data);
}

static TextOutputDev *
poppler_page_get_text_output_dev (PopplerPage *page)
{
  if (page->text_dev == NULL) {
    page->text_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);

    page->gfx = page->page->createGfx(page->text_dev,
				      72.0, 72.0, 0,
				      gFalse, /* useMediaBox */
				      gTrue, /* Crop */
				      -1, -1, -1, -1,
				      NULL, /* links */
				      page->document->doc->getCatalog (),
				      NULL, NULL, NULL, NULL);

    page->page->display(page->gfx);

    page->text_dev->endPage();
  }

  return page->text_dev;
}

/**
 * poppler_page_get_selection_region:
 * @page: a #PopplerPage
 * @scale: scale specified as pixels per point
 * @selection: start and end point of selection as a rectangle
 * 
 * Returns a region containing the area that would be rendered by
 * poppler_page_render_selection().  The returned region must be freed with
 * gdk_region_destroy().
 * 
 * Return value: a newly allocated #GdkRegion
 **/
GdkRegion *
poppler_page_get_selection_region (PopplerPage      *page,
				   gdouble           scale,
				   PopplerRectangle *selection)
{
  TextOutputDev *text_dev;
  PDFRectangle poppler_selection;
  GooList *list;
  GdkRectangle rect;
  GdkRegion *region;
  int i;

  poppler_selection.x1 = selection->x1;
  poppler_selection.y1 = selection->y1;
  poppler_selection.x2 = selection->x2;
  poppler_selection.y2 = selection->y2;

  text_dev = poppler_page_get_text_output_dev (page);
  list = text_dev->getSelectionRegion(&poppler_selection, scale);

  region = gdk_region_new();

  for (i = 0; i < list->getLength(); i++) {
    PDFRectangle *selection_rect = (PDFRectangle *) list->get(i);
    rect.x      = (gint) selection_rect->x1;
    rect.y      = (gint) selection_rect->y1;
    rect.width  = (gint) (selection_rect->x2 - selection_rect->x1);
    rect.height = (gint) (selection_rect->y2 - selection_rect->y1);
    gdk_region_union_with_rect (region, &rect);
    delete selection_rect;
  }

  delete list;

  return region;
}

#if defined (HAVE_CAIRO)

static void
poppler_page_set_selection_alpha (PopplerPage      *page,
				  double            scale,
				  GdkPixbuf        *pixbuf,
				  PopplerRectangle *selection)
{
  /* Cairo doesn't need this, since cairo generates an alpha channel. */ 
}

#elif defined (HAVE_SPLASH)

static void
poppler_page_set_selection_alpha (PopplerPage      *page,
				  double            scale,
				  GdkPixbuf        *pixbuf,
				  PopplerRectangle *selection)
{
  GdkRegion *region;
  gint n_rectangles, i, x, y, width, height;
  GdkRectangle *rectangles;
  int pixbuf_rowstride, pixbuf_n_channels;
  guchar *pixbuf_data, *dst;

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  if (pixbuf_n_channels != 4)
    return;

  for (y = 0; y < height; y++) {
      dst = pixbuf_data + y * pixbuf_rowstride;
      for (x = 0; x < width; x++) {
	  dst[3] = 0x00;
	  dst += pixbuf_n_channels;
      }
  }

  region = poppler_page_get_selection_region (page, scale, selection);

  gdk_region_get_rectangles (region, &rectangles, &n_rectangles);
  for (i = 0; i < n_rectangles; i++) {
    for (y = 0; y < rectangles[i].height; y++) {
      dst = pixbuf_data + (rectangles[i].y + y) * pixbuf_rowstride +
	rectangles[i].x * pixbuf_n_channels;
      for (x = 0; x < rectangles[i].width; x++) {
	  dst[3] = 0xff;
	  dst += pixbuf_n_channels;
      }
    }
  }

  g_free (rectangles);

  gdk_region_destroy (region);
}

#endif

/**
 * poppler_page_render_selection:
 * @page: the #PopplerPage for which to render selection
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render to
 * @selection: start and end point of selection as a rectangle
 * @old_selection: previous selection
 * @glyph_color: color to use for drawing glyphs
 * @background_color: color to use for the selection background
 * 
 * Render the selection specified by @selection for @page into
 * @pixbuf.  The selection will be rendered at @scale, using
 * @glyph_color for the glyphs and @background_color for the selection
 * background.
 *
 * If non-NULL, @old_selection specifies the selection that is already
 * rendered in @pixbuf, in which case this function will (some day)
 * only render the changed part of the selection.
 **/
void
poppler_page_render_selection (PopplerPage      *page,
			       gdouble           scale,
			       int		 rotation,
			       GdkPixbuf        *pixbuf,
			       PopplerRectangle *selection,
			       PopplerRectangle *old_selection,
			       GdkColor         *glyph_color,
			       GdkColor         *background_color)
{
  TextOutputDev *text_dev;
  OutputDev *output_dev;
  OutputDevData data;
  PDFRectangle pdf_selection(selection->x1, selection->y1,
			     selection->x2, selection->y2);

  GfxColor gfx_background_color = { 
      {
	  background_color->red,
	  background_color->green,
	  background_color->blue
      }
  };
  GfxColor gfx_glyph_color = {
      {
	  glyph_color->red,
	  glyph_color->green,
	  glyph_color->blue
      }
  };

  text_dev = poppler_page_get_text_output_dev (page);
  output_dev = page->document->output_dev;

  poppler_page_prepare_output_dev (page, scale, rotation, TRUE, &data);

  text_dev->drawSelection (output_dev, scale, rotation, &pdf_selection,
			   &gfx_glyph_color, &gfx_background_color);

  poppler_page_copy_to_pixbuf (page, pixbuf, &data);

  poppler_page_set_selection_alpha (page, scale, pixbuf, selection);

  /* We'll need a function to destroy page->text_dev and page->gfx
   * when the application wants to get rid of them.
   *
   * Two improvements: 1) make GfxFont refcounted and let TextPage and
   * friends hold a reference to the GfxFonts they need so we can free
   * up Gfx early.  2) use a TextPage directly when rendering the page
   * so we don't have to use TextOutputDev and render a second
   * time. */
}


static void
destroy_thumb_data (guchar *pixels, gpointer data)
{
  gfree (pixels);
}

/**
 * poppler_page_get_thumbnail:
 * @page: the #PopperPage to get the thumbnail for
 * 
 * Get the embedded thumbnail for the specified page.  If the document
 * doesn't have an embedded thumbnail for the page, this function
 * returns %NULL.
 * 
 * Return value: the tumbnail as a #GdkPixbuf or %NULL if the document
 * doesn't have a thumbnail for this page.
 **/
GdkPixbuf *
poppler_page_get_thumbnail (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);

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
 * Returns %TRUE if @page has a thumbnail associated with it.  It also
 * fills in @width and @height with the width and height of the
 * thumbnail.  The values of width and height are not changed if no
 * appropriate thumbnail exists.
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

  /* Theoretically, this could succeed and you would still fail when
   * loading the thumb */
  if (dict->lookupInt ("Width", "W", width)  &&
      dict->lookupInt ("Height", "H", height))
    retval = TRUE;

  thumb.free ();

  return retval;
}

/**
 * poppler_page_get_text:
 * @page: a #PopplerPage
 * @rect: the rectangle including the text
 * 
 * Retrieves the contents of the specified rectangle as text 
 * 
 * Return value: a pointer to the contents of the rectangle
 *               as a string
 **/
char *
poppler_page_get_text (PopplerPage      *page,
		       PopplerRectangle *selection)
{
  TextOutputDev *text_dev;
  PDFDoc *doc;
  GooString *sel_text;
  double height, y1, y2;
  char *result;
  PDFRectangle pdf_selection;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (selection != NULL, NULL);

  text_dev = poppler_page_get_text_output_dev (page);
  poppler_page_get_size (page, NULL, &height);

  pdf_selection.x1 = selection->x1;
  pdf_selection.y1 = height - selection->y2;
  pdf_selection.x2 = selection->x2;
  pdf_selection.y2 = height - selection->y1;
  
  sel_text = text_dev->getSelectionText (&pdf_selection);
  result = g_strdup (sel_text->getCString ());
  delete sel_text;

  return result;
}

/**
 * poppler_page_find_text:
 * @page: a #PopplerPage
 * @text: the text to search for (UTF-8 encoded)
 * 
 * A #GList of rectangles for each occurance of the text on the page.
 * The coordinates are in PDF points.
 * 
 * Return value: a #GList of PopplerRectangle, 
 **/
GList *
poppler_page_find_text (PopplerPage *page,
			const char  *text)
{
  PopplerRectangle *match;
  TextOutputDev *output_dev;
  PDFDoc *doc;
  GList *matches;
  double xMin, yMin, xMax, yMax;
  gunichar *ucs4;
  glong ucs4_len;
  double height;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (text != NULL, FALSE);

  ucs4 = g_utf8_to_ucs4_fast (text, -1, &ucs4_len);

  output_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);
  doc = page->document->doc;

  poppler_page_get_size (page, NULL, &height);
  page->page->display (output_dev, 72, 72, 0, gFalse,
		       gTrue, NULL, doc->getCatalog());
  
  matches = NULL;
  xMin = 0;
  yMin = 0;

  while (output_dev->findText (ucs4, ucs4_len,
			       gFalse, gTrue, // startAtTop, stopAtBottom
			       gTrue, gFalse, // startAtLast, stopAtLast
			       gFalse, gFalse, // caseSensitive, backwards
			       &xMin, &yMin, &xMax, &yMax))
    {
      match = g_new (PopplerRectangle, 1);
      match->x1 = xMin;
      match->y1 = height - yMax;
      match->x2 = xMax;
      match->y2 = height - yMin;
      matches = g_list_prepend (matches, match);
    }

  delete output_dev;
  g_free (ucs4);

  return g_list_reverse (matches);
}

/**
 * poppler_page_render_to_ps:
 * @page: a #PopplerPage
 * @ps_file: the PopplerPSFile to render to
 * 
 * Render the page on a postscript file
 * 
 **/
void
poppler_page_render_to_ps (PopplerPage   *page,
			   PopplerPSFile *ps_file)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (ps_file != NULL);

  if (!ps_file->out)
    ps_file->out = new PSOutputDev (ps_file->filename,
                                    ps_file->document->doc->getXRef(),
                                    ps_file->document->doc->getCatalog(),
                                    ps_file->first_page, ps_file->last_page,
                                    psModePS, (int)ps_file->paper_width,
                                    (int)ps_file->paper_height, ps_file->duplex,
                                    0, 0, 0, 0, gFalse);


  ps_file->document->doc->displayPage (ps_file->out, page->index + 1, 72.0, 72.0,
				       0, gFalse, gTrue, gFalse);
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
      g_value_take_string (value, _poppler_goo_string_to_utf8(&label));
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


/**
 * poppler_page_get_link_mapping:
 * @page: A #PopplerPage
 * 
 * Returns a list of #PopplerLinkMapping items that map from a
 * location on @page to a #PopplerAction.  This list must be freed
 * with poppler_page_free_link_mapping() when done.
 * 
 * Return value: A #GList of #PopplerLinkMapping
 **/
GList *
poppler_page_get_link_mapping (PopplerPage *page)
{
	GList *map_list = NULL;
	gint i;
	Links *links;
	Object obj;

	g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

	links = new Links (page->page->getAnnots (&obj),
			   page->document->doc->getCatalog ()->getBaseURI ());
	obj.free ();

	if (links == NULL)
		return NULL;

	for (i = 0; i < links->getNumLinks (); i++) {
		PopplerLinkMapping *mapping;
		LinkAction *link_action;
		Link *link;

		link = links->getLink (i);
		link_action = link->getAction ();

		/* Create the mapping */
		mapping = g_new (PopplerLinkMapping, 1);
		mapping->action = _poppler_action_new (page->document, link_action, NULL);
		link->getRect (&(mapping->area.x1), &(mapping->area.y1),
			       &(mapping->area.x2), &(mapping->area.y2));

		mapping->area.x1 -= page->page->getCropBox()->x1;
		mapping->area.x2 -= page->page->getCropBox()->x1;
		mapping->area.y1 -= page->page->getCropBox()->y1;
		mapping->area.y2 -= page->page->getCropBox()->y1;

		map_list = g_list_prepend (map_list, mapping);
	}

	delete links;

	return map_list;
}

static void
poppler_mapping_free (PopplerLinkMapping *mapping)
{
	poppler_action_free (mapping->action);
	g_free (mapping);
}

/**
 * poppler_page_free_link_mapping:
 * @list: A list of #PopplerLinkMapping<!-- -->s
 * 
 * Frees a list of #PopplerLinkMapping<!-- -->s allocated by
 * poppler_page_get_link_mapping().  It also frees the #PopplerAction<!-- -->s
 * that each mapping contains, so if you want to keep them around, you need to
 * copy them with poppler_action_copy().
 **/
void
poppler_page_free_link_mapping (GList *list)
{
	if (list == NULL)
		return;

	g_list_foreach (list, (GFunc) (poppler_mapping_free), NULL);
	g_list_free (list);
}

/* PopplerRectangle type */

GType
poppler_rectangle_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    our_type = g_boxed_type_register_static ("PopplerRectangle",
					     (GBoxedCopyFunc) poppler_rectangle_copy,
					     (GBoxedFreeFunc) poppler_rectangle_free);

  return our_type;
}

PopplerRectangle *
poppler_rectangle_new (void)
{
	return g_new0 (PopplerRectangle, 1);
}

PopplerRectangle *
poppler_rectangle_copy (PopplerRectangle *rectangle)
{
	PopplerRectangle *new_rectangle;

	g_return_val_if_fail (rectangle != NULL, NULL);

	new_rectangle = g_new0 (PopplerRectangle, 1);
	*new_rectangle = *rectangle;

	return new_rectangle;
}

void
poppler_rectangle_free (PopplerRectangle *rectangle)
{
	g_free (rectangle);
}

/* PopplerLinkMapping type */
GType
poppler_link_mapping_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    our_type = g_boxed_type_register_static ("PopplerLinkMapping",
					     (GBoxedCopyFunc) poppler_link_mapping_copy,
					     (GBoxedFreeFunc) poppler_link_mapping_free);

  return our_type;
}

PopplerLinkMapping *
poppler_link_mapping_new (void)
{
	return (PopplerLinkMapping *) g_new0 (PopplerLinkMapping, 1);
}

PopplerLinkMapping *
poppler_link_mapping_copy (PopplerLinkMapping *mapping)
{
	PopplerLinkMapping *new_mapping;

	new_mapping = poppler_link_mapping_new ();
	
	*new_mapping = *mapping;
	if (new_mapping->action)
		new_mapping->action = poppler_action_copy (new_mapping->action);

	return new_mapping;
}

void
poppler_link_mapping_free (PopplerLinkMapping *mapping)
{
	if (mapping)
		poppler_action_free (mapping->action);

	g_free (mapping);
}
