#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "poppler.h"

#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)


static void
print_document_info (PopplerDocument *document)
{
  gchar *title, *format, *author, *subject, *keywords;
  PopplerPageLayout layout;
  PopplerPageMode mode;
  PopplerViewerPreferences view_prefs;
  GEnumValue *enum_value;

  g_object_get (document,
		"title", &title,
		"format", &format,
		"author", &author,
		"subject", &subject,
		"keywords", &keywords,
		"page-mode", &mode,
		"page-layout", &layout,
		"viewer-preferences", &view_prefs,
		NULL);

  printf ("document metadata\n");
  if (title)  printf   ("\ttitle:\t%s\n", title);
  if (format) printf   ("\tformat:\t%s\n", format);
  if (author) printf   ("\tauthor:\t%s\n", author);
  if (subject) printf  ("\tsubject:\t%s\n", subject);
  if (keywords) printf ("\tdkeywords:\t%s\n", keywords);

  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_MODE), mode);
  g_print ("\tpage mode:\t%s\n", enum_value->value_name);
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_LAYOUT), layout);
  g_print ("\tpage layout:\t%s\n", enum_value->value_name);

  /* FIXME: print out the view prefs when we support it */

  g_free (title);
  g_free (format);
  g_free (author);
  g_free (subject);
  g_free (keywords);
}

int main (int argc, char *argv[])
{
  PopplerDocument *document;
  PopplerPage *page;
  char *label;
  GError *error;
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  GList *list, *l;
  char *text;
  PopplerRectangle area;

  if (argc != 3)
    FAIL ("usage: test-poppler-glib FILE PAGE");

  g_type_init ();

  error = NULL;
  document = poppler_document_new_from_file (argv[1], NULL, &error);
  if (document == NULL)
    FAIL (error->message);
      
  print_document_info (document); 

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

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 220, 220);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 100, 100, 200, 200, 1, pixbuf, 10, 10);

  gdk_pixbuf_save (pixbuf, "slice.png", "png", &error, NULL);
  printf ("saved 200x200 slice at (100, 100) as slice.png\n");
  if (error != NULL)
    FAIL (error->message);

  area.x1 = 0;
  area.y1 = 0;
  area.x2 = width;
  area.y2 = height;

  text = poppler_page_get_text (page, &area);
  if (text)
    {
      FILE *file = fopen ("dump.txt", "w");
      if (file)
	{
	  fwrite (text, strlen (text), 1, file);
	  fclose (file);
	}
      g_free (text);
    }

  list = poppler_page_find_text (page, "Bitwise");
  printf ("Found text \"Bitwise\" at positions:\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerRectangle *rect = l->data;

      printf ("  (%f,%f)-(%f,%f)\n", rect->x1, rect->y1, rect->x2, rect->y2);
    }
    

  g_object_unref (G_OBJECT (page));

  g_object_unref (G_OBJECT (document));

  return 0;
}
