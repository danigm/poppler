#include <stdio.h>
#include <stdlib.h>
#include "poppler.h"

#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

int main (int argc, char *argv[])
{
  PopplerDocument *document;
  PopplerPage *page;
  char *filename;
  GError *error;
  GdkPixbuf *pixbuf;
  double width, height;

  filename = argv[1];
  if (filename == NULL)
    FAIL ("specify filename");

  g_type_init ();

  error = NULL;
  document = poppler_document_new_from_file (argv[1], NULL, &error);
  if (document == NULL)
    FAIL (error->message);
      
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 100, 100);
  gdk_pixbuf_fill (pixbuf, 0x00106000);

  page = poppler_document_get_page (document, 5);
  poppler_page_get_dimensions (page, &width, &height);
  printf ("page dimensions: %f inches by %f inches\n",
	  width / 72, height / 72);
  poppler_page_render_to_pixbuf (page, 100, 100, 50, 50, 1, pixbuf, 10, 10);
  g_object_unref (G_OBJECT (page));

  gdk_pixbuf_save (pixbuf, "slice.png", "png", &error, NULL);
  if (error != NULL)
    FAIL (error->message);

  g_object_unref (G_OBJECT (document));
}
