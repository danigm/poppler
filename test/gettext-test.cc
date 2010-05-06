#include "config.h"
#include "Page.h"
#include <poppler-config.h>
#include "GlobalParams.h"
#include "Error.h"
#include "PDFDoc.h"
#include "goo/GooString.h"
#include "TextOutputDev.h"

int main (int argc, char *argv[])
{
  PDFDoc *doc;
  GooString *inputName;
  GooString *s;
  char *result;
  int page_index;
  TextOutputDev *textOut;
  Page *page;
  PDFRectangle *rect;

  // parse args
  if (argc < 3) {
    fprintf(stderr, "usage: %s INPUT-FILE page\n", argv[0]);
    return 1;
  }
  if (!sscanf (argv[2], "%d", &page_index))
  {
    fprintf(stderr, "usage: %s INPUT-FILE page\n", argv[0]);
    return 1;
  }

  inputName = new GooString(argv[1]);

  globalParams = new GlobalParams();

  doc = new PDFDoc(inputName);

  if (!doc->isOk()) {
    delete doc;
    fprintf(stderr, "Error loading document !\n");
    return 1;
  }

  page = doc->getCatalog()->getPage(1);

  //textOut = new TextOutputDev(0, gFalse, gFalse, gFalse);
  textOut = new TextOutputDev(0, gTrue, gTrue, gFalse);
  doc->displayPageSlice(textOut, page_index, 72, 72,
      0, false, true, false, -1, -1, -1, -1);

  rect = page->getCropBox();
  s = textOut->getText(rect->x1, rect->y1, rect->x2, rect->y2);

  result = s->getCString ();
  printf ("%s\n", result);

  delete textOut;
  delete s;

  delete doc;
  delete globalParams;
  return 0;
}
