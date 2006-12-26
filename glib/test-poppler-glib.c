#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "poppler.h"

#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)


static void
print_index (PopplerIndexIter *iter)
{
  do
    {
      PopplerAction *action;
      PopplerIndexIter *child;

      action = poppler_index_iter_get_action (iter);
      g_print ("Action: %d\n", action->type);
      poppler_action_free (action);
      child = poppler_index_iter_get_child (iter);
      if (child)
	print_index (child);
      poppler_index_iter_free (child);
    }
  while (poppler_index_iter_next (iter));
}

static void
print_document_info (PopplerDocument *document)
{
  gchar *title, *format, *author, *subject, *keywords, *creator, *producer, *linearized;
  GTime creation_date, mod_date;
  PopplerPageLayout layout;
  PopplerPageMode mode;
  PopplerViewerPreferences view_prefs;
  PopplerFontInfo *font_info;
  PopplerFontsIter *fonts_iter;
  PopplerIndexIter *index_iter;
  GEnumValue *enum_value;

  g_object_get (document,
		"title", &title,
		"format", &format,
		"author", &author,
		"subject", &subject,
		"keywords", &keywords,
		"creation-date", &creation_date,
		"mod-date", &mod_date,
		"creator", &creator,
		"producer", &producer,	
		"linearized", &linearized,
		"page-mode", &mode,
		"page-layout", &layout,
		"viewer-preferences", &view_prefs,
		NULL);

  printf ("\t---------------------------------------------------------\n");
  printf ("\tDocument Metadata\n");
  printf ("\t---------------------------------------------------------\n");
  if (title)  printf   ("\ttitle:\t\t%s\n", title);
  if (format) printf   ("\tformat:\t\t%s\n", format);
  if (author) printf   ("\tauthor:\t\t%s\n", author);
  if (subject) printf  ("\tsubject:\t%s\n", subject);
  if (keywords) printf ("\tkeywords:\t%s\n", keywords);
  if (creator) printf ("\tcreator:\t%s\n", creator);
  if (producer) printf ("\tproducer:\t%s\n", producer);
  if (linearized) printf ("\tlinearized:\t%s\n", linearized);
  
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_MODE), mode);
  g_print ("\tpage mode:\t%s\n", enum_value->value_name);
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_LAYOUT), layout);
  g_print ("\tpage layout:\t%s\n", enum_value->value_name);

  g_print ("\tcreation date:\t%d\n", creation_date);
  g_print ("\tmodified date:\t%d\n", mod_date);

  g_print ("\tfonts:\n");
  font_info = poppler_font_info_new (document);
  while (poppler_font_info_scan (font_info, 20, &fonts_iter)) {
    if (fonts_iter) {
      do {
        g_print ("\t\t\t%s\n", poppler_fonts_iter_get_name (fonts_iter));
      } while (poppler_fonts_iter_next (fonts_iter));
      poppler_fonts_iter_free (fonts_iter);
    }
  }
  poppler_font_info_free (font_info);

  index_iter = poppler_index_iter_new (document);
  if (index_iter)
    {
      g_print ("\tindex:\n");
      print_index (index_iter);
      poppler_index_iter_free (index_iter);
    }
  

  
  /* FIXME: print out the view prefs when we support it */

  g_free (title);
  g_free (format);
  g_free (author);
  g_free (subject);
  g_free (keywords);
  g_free (creator);
  g_free (producer); 
  g_free (linearized);
}

int main (int argc, char *argv[])
{
  PopplerDocument *document;
  PopplerBackend backend;
  PopplerPage *page;
  GEnumValue *enum_value;
  char *label;
  GError *error;
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  GList *list, *l;
  char *text;
  double duration;
  PopplerRectangle area;

  if (argc != 3)
    FAIL ("usage: test-poppler-glib file://FILE PAGE");

  g_type_init ();

  g_print ("Poppler version %s\n", poppler_get_version ());
  backend = poppler_get_backend ();
  enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_BACKEND), backend);
  g_print ("Backend is %s\n", enum_value->value_name);

  error = NULL;
  document = poppler_document_new_from_file (argv[1], NULL, &error);
  if (document == NULL)
    FAIL (error->message);

  print_document_info (document); 

  page = poppler_document_get_page_by_label (document, argv[2]);
  if (page == NULL)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);
  printf ("\tpage size:\t%f inches by %f inches\n", width / 72, height / 72);

  duration = poppler_page_get_duration (page);
  if (duration > 0)
    printf ("\tpage duration:\t%f second(s)\n", duration);
  else
    printf ("\tpage duration:\tno duration for page\n");

  thumb = poppler_page_get_thumbnail (page);
  if (thumb != NULL) {
    gdk_pixbuf_save (thumb, "thumb.png", "png", &error, NULL);
    if (error != NULL)
      FAIL (error->message);
    else
      printf ("\tthumbnail:\tsaved as thumb.png\n");
    g_object_unref (G_OBJECT (thumb));
  }
  else
    printf ("\tthumbnail:\tno thumbnail for page\n");

  g_object_get (page, "label", &label, NULL);
  printf ("\tpage label:\t%s\n", label);
  g_free (label);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 220, 220);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 100, 100, 200, 200, 1, 0, pixbuf);

  gdk_pixbuf_save (pixbuf, "slice.png", "png", &error, NULL);
  printf ("\tslice:\t\tsaved 200x200 slice at (100, 100) as slice.png\n");
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
  printf ("\n");  
  printf ("\tFound text \"Bitwise\" at positions:\n");
  for (l = list; l != NULL; l = l->next)
    {
      PopplerRectangle *rect = l->data;

      printf ("  (%f,%f)-(%f,%f)\n", rect->x1, rect->y1, rect->x2, rect->y2);
    }

  if (poppler_document_has_attachments (document))
    {
      int i = 0;

      g_print ("Attachments found:\n\n");

      list = poppler_document_get_attachments (document);
      for (l = list; l; l = l->next)
	{
	  PopplerAttachment *attachment;
	  char *name;

	  name = g_strdup_printf ("/tmp/attach%d", i);
	  attachment = l->data;
	  g_print ("\tname: %s\n", attachment->name);
	  g_print ("\tdescription: %s\n\n", attachment->description);
	  poppler_attachment_save (attachment, name, NULL);
	  i++;
	}
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
  else
    g_print ("no attachment\n");

  g_object_unref (G_OBJECT (page));

  g_object_unref (G_OBJECT (document));

  return 0;
}
