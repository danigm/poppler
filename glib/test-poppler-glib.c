#include <stdio.h>
#include <stdlib.h>
#include "poppler.h"

#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

int main (int argc, char *argv[])
{
  PopplerDocument *document;
  PopplerPage *page;
  char *title, *label;
  GError *error;
  GdkPixbuf *pixbuf, *thumb;
  double width, height;

  if (argc != 3)
    FAIL ("usage: test-poppler-glib FILE PAGE");

  g_type_init ();

  error = NULL;
  document = poppler_document_new_from_file (argv[1], NULL, &error);
  if (document == NULL)
    FAIL (error->message);
      
  g_object_get (document, "title", &title, NULL);
  printf ("document title: %s\n", title);
  g_free (title);

  page = poppler_document_get_page_by_label (document, argv[2]);
  if (page == NULL)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);
  printf ("page size: %f inches by %f inches\n", width / 72, height / 72);

  thumb = poppler_page_get_thumbnail (page);
  if (thumb != NULL) {
    gdk_pixbuf_save (thumb, "thumb.png", "png", &error, NULL);
    if (error != NULL)
      FAIL (error->message);
    else
      printf ("saved thumbnail as thumb.png\n");
    g_object_unref (G_OBJECT (thumb));
  }
  else
    printf ("no thumbnail for page\n");

  g_object_get (page, "label", &label, NULL);
  printf ("page label: %s\n", label);
  g_free (label);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 100, 100);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 100, 100, 50, 50, 1, pixbuf, 10, 10);
  g_object_unref (G_OBJECT (page));

  gdk_pixbuf_save (pixbuf, "slice.png", "png", &error, NULL);
  if (error != NULL)
    FAIL (error->message);

  g_object_unref (G_OBJECT (document));

  return 0;
}
