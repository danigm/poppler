#ifndef __POPPLER_PRIVATE_H__
#define __POPPLER_PRIVATE_H__

#include <config.h>
#include <PDFDoc.h>
#include <PSOutputDev.h>
#include <Link.h>

#if defined (HAVE_CAIRO)
#include <CairoOutputDevImage.h>
#elif defined (HAVE_SPLASH)
#include <SplashOutputDev.h>
#endif

struct _PopplerDocument
{
  GObject parent_instance;
  PDFDoc *doc;

#if defined (HAVE_CAIRO)
  CairoOutputDevImage *output_dev;
#elif defined (HAVE_SPLASH)
  SplashOutputDev *output_dev;
#endif
};

struct _PopplerPSFile
{
  PopplerDocument *document;
  PSOutputDev *out;
};

struct _PopplerPage
{
  GObject parent_instance;
  PopplerDocument *document;
  Page *page;
  int index;
  PopplerOrientation orientation;
};

PopplerPage   *_poppler_page_new   (PopplerDocument *document,
				    Page            *page,
				    int              index);
PopplerAction *_poppler_action_new (PopplerDocument *document,
				    LinkAction      *link,
				    const gchar     *title);


#endif
