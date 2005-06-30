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

static PopplerOrientation
get_document_orientation (PopplerPage *page)
{
  PopplerOrientation orientation;
  int rotation = page->page->getRotate ();

  switch (rotation) {
    case 90:
      orientation = POPPLER_ORIENTATION_LANDSCAPE;
      break;
    case 180:
      orientation = POPPLER_ORIENTATION_UPSIDEDOWN;
      break;
    case 270:
      orientation = POPPLER_ORIENTATION_SEASCAPE;
      break;
    default:
      orientation = POPPLER_ORIENTATION_PORTRAIT;
  }

  return orientation;
}

PopplerPage *
_poppler_page_new (PopplerDocument *document, Page *page, int index)
{
  PopplerPage *poppler_page;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

  poppler_page = (PopplerPage *) g_object_new (POPPLER_TYPE_PAGE, NULL);
  poppler_page->document = document;
  poppler_page->page = page;
  poppler_page->index = index;
  poppler_page->orientation = get_document_orientation (poppler_page);

  return poppler_page;
}

static void
poppler_page_finalize (GObject *object)
{
  PopplerPage *page = POPPLER_PAGE (object);

  if (page->gfx != NULL)
    delete page->gfx;
  if (page->text_dev != NULL)
    delete page->text_dev;

  /* page->page is owned by the document */
}

static int
poppler_page_get_rotate (PopplerPage *page)
{
  int rotate;

  switch (page->orientation) {
    case POPPLER_ORIENTATION_PORTRAIT:
      rotate = 0;
      break;
    case POPPLER_ORIENTATION_LANDSCAPE:
      rotate = 90;
      break;
    case POPPLER_ORIENTATION_UPSIDEDOWN:
      rotate = 180;
      break;
    case POPPLER_ORIENTATION_SEASCAPE:
      rotate = 270;
      break;
    default:
      rotate = page->page->getRotate ();
  }

  return rotate - page->page->getRotate ();
}

void
poppler_page_get_size (PopplerPage *page,
		       double      *width,
		       double      *height)
{
  double page_width, page_height;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  switch (page->orientation) {
    case POPPLER_ORIENTATION_PORTRAIT:
    case POPPLER_ORIENTATION_UPSIDEDOWN:
      page_width = page->page->getWidth ();
      page_height = page->page->getHeight ();
      break;
    case POPPLER_ORIENTATION_LANDSCAPE:
    case POPPLER_ORIENTATION_SEASCAPE:
      page_width = page->page->getHeight ();
      page_height = page->page->getWidth ();
      break;
    default:
      page_width = page_height = 0;
      g_assert_not_reached ();
  }

  if (width != NULL)
    *width = page_width;
  if (height != NULL)
    *height = page_height;
}

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
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  CairoOutputDev *output_dev;
  cairo_surface_t *surface;
  int cairo_width, cairo_height, cairo_rowstride;
  unsigned char *cairo_data;

  output_dev = page->document->output_dev;
  cairo_width = MAX ((int)(page->page->getWidth() * scale + 0.5), 1);
  cairo_height = MAX ((int)(page->page->getHeight() * scale + 0.5), 1);
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
  output_dev->setSurface (surface);
}

void
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

  page->document->output_dev->setSurface (NULL);
  cairo_surface_destroy (output_dev_data->surface);
  gfree (output_dev_data->cairo_data);
}

#elif defined (HAVE_SPLASH)

typedef struct {
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  /* pft */
}

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

  if (dest_x + splash_width > gdk_pixbuf_get_width (pixbuf))
    splash_width = gdk_pixbuf_get_width (pixbuf) - dest_x;
  if (dest_y + splash_height > gdk_pixbuf_get_height (pixbuf))
    splash_height = gdk_pixbuf_get_height (pixbuf) - dest_y;

  for (y = 0; y < splash_height; y++)
    {
      SplashRGB8 *src;

      src = (SplashRGB8 *) (color_ptr.rgb8p + y * splash_rowstride);
      dst = pixbuf_data + (dest_y + y) * pixbuf_rowstride +
	dest_x * pixbuf_n_channels;
      for (x = 0; x < splash_width; x++) 
	{
	  dst[0] = splashRGB8R(*src);
	  dst[1] = splashRGB8G(*src); 
	  dst[2] = splashRGB8B(*src);
	  dst += pixbuf_n_channels;
	  src++;
	}
    }
}

#endif

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
  OutputDevData data;

  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  poppler_page_prepare_output_dev (page, scale, FALSE, &data);

  page->page->displaySlice(page->document->output_dev,
			   72.0 * scale, 72.0 * scale,
			   poppler_page_get_rotate (page),
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
				      72.0, 72.0,
				      poppler_page_get_rotate (page),
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

void
poppler_page_render_selection (PopplerPage *page,
			       gdouble      scale,
			       GdkPixbuf   *pixbuf,
			       PopplerRectangle *selection,
			       PopplerRectangle *old_selection)
{
  TextOutputDev *text_dev;
  CairoOutputDev *output_dev;
  cairo_surface_t *surface;
  unsigned char *cairo_data;
  OutputDevData data;
  PDFRectangle pdf_selection(selection->x1, selection->y1,
			     selection->x2, selection->y2);

  text_dev = poppler_page_get_text_output_dev (page);
  output_dev = page->document->output_dev;

  poppler_page_prepare_output_dev (page, scale, TRUE, &data);

  text_dev->drawSelection (output_dev, scale, &pdf_selection);

  poppler_page_copy_to_pixbuf (page, pixbuf, &data);

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
		       PopplerRectangle *rect)
{
  TextOutputDev *output_dev;
  PDFDoc *doc;
  GooString *sel_text = new GooString;
  double height, y1, y2;
  char *result;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (rect != NULL, NULL);

  output_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);
  doc = page->document->doc;

  height = page->page->getHeight ();
  page->page->display(output_dev, 72, 72, poppler_page_get_rotate (page),
		      gTrue, NULL, doc->getCatalog());

  y1 = height - rect->y2;
  y2 = height - rect->y1;
  sel_text = output_dev->getText (rect->x1, y1, rect->x2, y2);
  result = sel_text->getCString ();

  delete output_dev;

  return result ? g_strdup (result) : NULL;
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

  height = page->page->getHeight ();
  page->page->display (output_dev, 72, 72, poppler_page_get_rotate (page),
		       gTrue, NULL, doc->getCatalog());
  
  matches = NULL;
  xMin = 0;
  yMin = 0;
  while (output_dev->findText (ucs4, ucs4_len,
			       gFalse, gTrue, // startAtTop, stopAtBottom
			       gTrue, gFalse, // startAtLast, stopAtLast
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

  ps_file->document->doc->displayPage (ps_file->out, page->index + 1, 72.0, 72.0,
				       poppler_page_get_rotate (page), gTrue, gFalse);
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

		mapping->area.x1 -= page->page->getBox()->x1;
		mapping->area.x2 -= page->page->getBox()->x1;
		mapping->area.y1 -= page->page->getBox()->y1;
		mapping->area.y2 -= page->page->getBox()->y1;

		map_list = g_list_prepend (map_list, mapping);
	}

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

/**
 * poppler_page_set_orientation:
 * @page: a #PopplerPage
 * @orientation: a #PopplerOrientation
 *
 * Force the orientation of the page to be the specified one
 *
 **/
void
poppler_page_set_orientation (PopplerPage        *page,
			      PopplerOrientation  orientation)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));

  page->orientation = orientation;
}

/**
 * poppler_page_get_orientation:
 * @page: a #PopplerPage
 * @orientation: a #PopplerOrientation
 *
 * Return the orientation of the specified page
 *
 * Return value: a #PopplerOrientation
 **/
PopplerOrientation
poppler_page_get_orientation (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), POPPLER_ORIENTATION_PORTRAIT);

  return page->orientation;
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
