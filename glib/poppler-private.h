#ifndef __POPPLER_PRIVATE_H__
#define __POPPLER_PRIVATE_H__

struct _PopplerDocument
{
  GObject parent_instance;
  PDFDoc *doc;
};

struct _PopplerPage
{
  GObject parent_instance;
  PopplerDocument *document;
  Page *page;
  int index;
};

PopplerPage *
_poppler_page_new (PopplerDocument *document, Page *page, int index);

#endif
