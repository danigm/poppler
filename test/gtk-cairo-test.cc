//========================================================================
//
// GDKSplashOutputDev.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc. (GDK port)
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <goo/gmem.h>
#include <splash/SplashTypes.h>
#include <splash/SplashBitmap.h>
#include "Object.h"
#include "GfxState.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "CairoOutputDev.h"
#include <cairo-xlib.h>
#include <X11/Xutil.h>

#include "PDFDoc.h"
#include "GlobalParams.h"
#include "ErrorCodes.h"
#include <gtk/gtk.h>

class GDKCairoOutputDev: public CairoOutputDev {
public:

  GDKCairoOutputDev(GdkDrawable *drawable,
		    void (*redrawCbkA)(void *data),
		    void *redrawCbkDataA);
  
  virtual ~GDKCairoOutputDev();

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  //----- special access

  // Clear out the document (used when displaying an empty window).
  void clear();

  // Copy the rectangle (srcX, srcY, width, height) to (destX, destY)
  // in destDC.
  void redraw(int srcX, int srcY,
              GdkDrawable *drawable,
	      int destX, int destY,
	      int width, int height);

  int getPixmapWidth (void) { return pixmapWidth; }
  int getPixmapHeight (void) { return pixmapHeight; }

private:

  int incrementalUpdate;
  void (*redrawCbk)(void *data);
  void *redrawCbkData;
  int pixmapWidth, pixmapHeight;
  GdkPixmap *pixmap, *drawable;
};

GDKCairoOutputDev::GDKCairoOutputDev(GdkDrawable *drawableA,
				     void (*redrawCbkA)(void *data),
				     void *redrawCbkDataA):
  CairoOutputDev()
{
  drawable = drawableA;
  redrawCbk = redrawCbkA;
  redrawCbkData = redrawCbkDataA;
  pixmap = NULL;
}

GDKCairoOutputDev::~GDKCairoOutputDev() {
}

void
GDKCairoOutputDev::startPage(int pageNum, GfxState *state) {
  Display *display;
  Drawable xid;
  GdkGC *gc;
  GdkColor white;
  cairo_surface_t *surface;
  int w, h;

  w = state ? (int)(state->getPageWidth() + 0.5) : 1;
  h = state ? (int)(state->getPageHeight() + 0.5) : 1;

  if (!pixmap || pixmapWidth != w || h != pixmapHeight != h) {
    if (pixmap)
      g_object_unref (G_OBJECT (pixmap));

    pixmap = gdk_pixmap_new (drawable, w, h, -1);
    pixmapWidth = w;
    pixmapHeight = h;

    gc = gdk_gc_new (pixmap);
    white.red = 0xffff;
    white.green = 0xffff;
    white.blue = 0xffff;
    gdk_gc_set_rgb_fg_color (gc, &white);
    gdk_draw_rectangle (pixmap, gc, TRUE, 0, 0, w, h);
    g_object_unref (G_OBJECT (gc));
  }

  if (pixmap) {
    display = gdk_x11_drawable_get_xdisplay (pixmap);
    xid = gdk_x11_drawable_get_xid (pixmap);

    surface = cairo_xlib_surface_create(display, xid,
					DefaultVisual(display, DefaultScreen(display)),
					w, h);
    setSurface(surface);
    cairo_surface_destroy (surface);
  }

  CairoOutputDev::startPage(pageNum, state);
}

void GDKCairoOutputDev::redraw(int srcX, int srcY,
			       GdkDrawable *drawable,
			       int destX, int destY,
			       int width, int height) {
  GdkGC *gc;

  gc = gdk_gc_new (drawable);
  gdk_draw_drawable (drawable, gc,
		     pixmap, srcX, srcY,
		     destX, destY, width, height);
  g_object_unref (gc);
}

typedef struct
{
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *drawing_area;
  GDKCairoOutputDev *out;
  PDFDoc *doc;
} View;

static void
drawing_area_expose (GtkWidget      *drawing_area,
                     GdkEventExpose *event,
                     void           *data)
{
  View *v = (View*) data;
  GdkRectangle document;
  GdkRectangle draw;

  gdk_window_clear (drawing_area->window);
  
  document.x = 0;
  document.y = 0;
  document.width = v->out->getPixmapWidth();
  document.height = v->out->getPixmapHeight();

  if (gdk_rectangle_intersect (&document, &event->area, &draw))
    {
      v->out->redraw (draw.x, draw.y,
                      drawing_area->window,
                      draw.x, draw.y,
                      draw.width, draw.height);
    }
}

static int
view_load (View       *v,
           const char *filename)
{
  PDFDoc *newDoc;
  int err;
  GooString *filename_g;
  int w, h;

  filename_g = new GooString (filename);

  // open the PDF file
  newDoc = new PDFDoc(filename_g, 0, 0);

  delete filename_g;
  
  if (!newDoc->isOk())
    {
      err = newDoc->getErrorCode();
      delete newDoc;
      return err;
    }

  if (v->doc)
    delete v->doc;
  v->doc = newDoc;
  
  v->out->startDoc(v->doc->getXRef());

  v->doc->displayPage (v->out, 1, 72, 72, 0, gFalse, gTrue, gTrue);
  
  w = v->out->getPixmapWidth();
  h = v->out->getPixmapHeight();
  
  gtk_widget_set_size_request (v->drawing_area, w, h);

  return errNone;
}

static void
view_show (View *v)
{
  gtk_widget_show (v->window);
}

static void
redraw_callback (void *data)
{
  View *v = (View*) data;

  gtk_widget_queue_draw (v->drawing_area);
}

static View*
view_new (void)
{
  View *v;
  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *sw;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  drawing_area = gtk_drawing_area_new ();

  sw = gtk_scrolled_window_new (NULL, NULL);

  gtk_container_add (GTK_CONTAINER (window), sw);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), drawing_area);

  gtk_widget_show_all (sw);
  gtk_widget_realize (window);

  v = g_new0 (View, 1);

  v->window = window;
  v->drawing_area = drawing_area;
  v->sw = sw;
  v->out = new GDKCairoOutputDev (window->window, redraw_callback, (void*) v);
  v->doc = 0;

  g_signal_connect (drawing_area,
                    "expose_event",
                    G_CALLBACK (drawing_area_expose),
                    (void*) v);
  
  return v;
}

int
main (int argc, char *argv [])
{
  View *v;
  int i;
  
  gtk_init (&argc, &argv);
  
  globalParams = new GlobalParams("/etc/xpdfrc");
  
  if (argc == 1)
    {
      fprintf (stderr, "usage: %s PDF-FILES...\n", argv[0]);
      return -1;
    }
      

  i = 1;
  while (i < argc)
    {
      int err;
      
      v = view_new ();

      err = view_load (v, argv[i]);

      if (err != errNone)
        g_printerr ("Error loading document!\n");
      
      view_show (v);

      ++i;
    }
  
  gtk_main ();
  
  delete globalParams;
  
  return 0;
}
