/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
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

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <ArthurOutputDev.h>
#include <SplashOutputDev.h>
#include <TextOutputDev.h>
#include <splash/SplashBitmap.h>

#include "poppler-private.h"
#include "poppler-page-transition-private.h"

namespace Poppler {

class PageData {
  public:
  const Document *parentDoc;
  int index;
  PageTransition *transition;
};

Page::Page(const Document *doc, int index) {
  m_page = new PageData();
  m_page->index = index;
  m_page->parentDoc = doc;
  m_page->transition = 0;
}

Page::~Page()
{
  delete m_page->transition;
  delete m_page;
}

QImage Page::splashRenderToImage(double xres, double yres, int x, int y, int w, int h, bool doLinks) const
{
  SplashOutputDev *output_dev = m_page->parentDoc->m_doc->getSplashOutputDev();
  
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, xres, yres,
						 0, false, true, doLinks, x, y, w, h);
  
  SplashBitmap *bitmap = output_dev->getBitmap ();
  int bw = bitmap->getWidth();
  int bh = bitmap->getHeight();
  
  SplashColorPtr dataPtr = output_dev->getBitmap()->getDataPtr();
  // construct a qimage SHARING the raw bitmap data in memory
  QImage img( dataPtr, bw, bh, QImage::Format_ARGB32 );
  img = img.copy();
  // unload underlying xpdf bitmap
  output_dev->startPage( 0, NULL );

  return img;
}

QPixmap *Page::splashRenderToPixmap(double xres, double yres, int x, int y, int w, int h, bool doLinks) const
{
  QImage img = splashRenderToImage(xres, yres, x, y, w, h, doLinks);

  // Turn the QImage into a QPixmap
  QPixmap* out = new QPixmap(QPixmap::fromImage(img));

  return out;
}

void Page::renderToPixmap(QPixmap *pixmap, double xres, double yres) const
{
  QPainter* painter = new QPainter(pixmap);
  painter->setRenderHint(QPainter::Antialiasing);
  ArthurOutputDev output_dev(painter);
  output_dev.startDoc(m_page->parentDoc->m_doc->doc.getXRef ());
  m_page->parentDoc->m_doc->doc.displayPageSlice(&output_dev,
						 m_page->index + 1,
						 xres,
						 yres,
						 0,
						 false,
						 true,
						 false,
						 -1,
						 -1,
						 -1,
						 -1);
  painter->end();
}

QString Page::text(const QRectF &r) const
{
  TextOutputDev *output_dev;
  GooString *s;
  PDFRectangle *rect;
  QString result;
  ::Page *p;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, true, false, -1, -1, -1, -1);
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if (r.isNull())
  {
    rect = p->getCropBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    double height, y1, y2;
    height = p->getCropHeight();
    y1 = height - r.top();
    y2 = height - r.bottom();
    s = output_dev->getText(r.left(), y1, r.right(), y2);
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

bool Page::search(const QString &text, QRectF &rect, SearchDirection direction, SearchMode caseSensitive) const
{
  const QChar * str = text.unicode();
  int len = text.length();
  QVector<Unicode> u(len);
  for (int i = 0; i < len; ++i) u[i] = str[i].unicode();

  GBool sCase;
  if (caseSensitive == CaseSensitive) sCase = gTrue;
  else sCase = gFalse;

  bool found = false;
  double sLeft, sTop, sRight, sBottom;
  sLeft = rect.left();
  sTop = rect.top();
  sRight = rect.right();
  sBottom = rect.bottom();

  // fetch ourselves a textpage
  TextOutputDev td(NULL, gTrue, gFalse, gFalse);
  m_page->parentDoc->m_doc->doc.displayPage( &td, m_page->index + 1, 72, 72, 0, false, true, false );
  TextPage *textPage=td.takeText();

  if (direction == FromTop)
    found = textPage->findText( u.data(), len, 
            gTrue, gTrue, gFalse, gFalse, sCase, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == NextResult )
    found = textPage->findText( u.data(), len, 
            gFalse, gTrue, gTrue, gFalse, sCase, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == PreviousResult )
    found = textPage->findText( u.data(), len, 
            gTrue, gFalse, gFalse, gTrue, sCase, gFalse, &sLeft, &sTop, &sRight, &sBottom );

  delete textPage;

  rect.setLeft( sLeft );
  rect.setTop( sTop );
  rect.setRight( sRight );
  rect.setBottom( sBottom );

  return found;
}

QList<TextBox*> Page::textList() const
{
  TextOutputDev *output_dev;
  
  QList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);

  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);

  TextWordList *word_list = output_dev->makeWordList();
  
  if (!word_list) {
    delete output_dev;
    return output_list;
  }
  
  QMap<TextWord *, TextBox*> wordBoxMap;
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    QString string = QString::fromUtf8(word->getText()->getCString());
    double xMin, yMin, xMax, yMax;
    word->getBBox(&xMin, &yMin, &xMax, &yMax);
    
    TextBox* text_box = new TextBox(string, QRectF(xMin, yMin, xMax-xMin, yMax-yMin));
    text_box->m_data->hasSpaceAfter = word->hasSpaceAfter() == gTrue;
    text_box->m_data->edge.reserve(word->getLength() + 1);
    for (int j = 0; j <= word->getLength(); ++j) text_box->m_data->edge.append(word->getEdge(j));
    
    wordBoxMap.insert(word, text_box);
    
    output_list.append(text_box);
  }
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    TextBox* text_box = wordBoxMap[word];
    text_box->m_data->nextWord = wordBoxMap[word->nextWord()];
  }
  
  delete word_list;
  delete output_dev;
  
  return output_list;
}

PageTransition *Page::transition() const
{
  if (!m_page->transition) {
    Object o;
    PageTransitionParams params;
    params.dictObj = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1)->getTrans(&o);
    if (params.dictObj->isDict()) m_page->transition = new PageTransition(params);
    o.free();
  }
  return m_page->transition;
}

QSizeF Page::pageSizeF() const
{
  ::Page *p;
  
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if ( ( Page::Landscape == orientation() ) || (Page::Seascape == orientation() ) ) {
      return QSizeF( p->getCropHeight(), p->getCropWidth() );
  } else {
    return QSizeF( p->getCropWidth(), p->getCropHeight() );
  }
}

QSize Page::pageSize() const
{
  return QSize( (int)pageSizeF().width(), (int)pageSizeF().height() );
}

Page::Orientation Page::orientation() const
{
  int rotation = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1)->getRotate();
  switch (rotation) {
  case 90:
    return Page::Landscape;
    break;
  case 180:
    return Page::UpsideDown;
    break;
  case 270:
    return Page::Seascape;
    break;
  default:
    return Page::Portrait;
  }
}

void Page::defaultCTM(double *CTM, double dpiX, double dpiY, int rotate, bool upsideDown)
{
  ::Page *p;
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  p->getDefaultCTM(CTM, dpiX, dpiY, rotate, upsideDown);
}

QList<Link*> Page::links() const
{
  QList<Link*> popplerLinks;

  Links *xpdfLinks = m_page->parentDoc->m_doc->doc.takeLinks();
  for (int i = 0; i < xpdfLinks->getNumLinks(); ++i)
  {
    ::Link *xpdfLink = xpdfLinks->getLink(i);
    
    double left, top, right, bottom;
    int leftAux, topAux, rightAux, bottomAux;
    xpdfLink->getRect( &left, &top, &right, &bottom );
    QRectF linkArea;
    
    m_page->parentDoc->m_doc->m_splashOutputDev->cvtUserToDev( left, top, &leftAux, &topAux );
    m_page->parentDoc->m_doc->m_splashOutputDev->cvtUserToDev( right, bottom, &rightAux, &bottomAux );
    linkArea.setLeft(leftAux);
    linkArea.setTop(topAux);
    linkArea.setRight(rightAux);
    linkArea.setBottom(bottomAux);

    if (!xpdfLink->isOk()) continue;

    Link *popplerLink = NULL;
    ::LinkAction *a = xpdfLink->getAction();
    if ( a )
    {
      switch ( a->getKind() )
      {
        case actionGoTo:
        {
          LinkGoTo * g = (LinkGoTo *) a;
          // create link: no ext file, namedDest, object pointer
          popplerLink = new LinkGoto( linkArea, QString::null, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), m_page->parentDoc->m_doc ) ) );
        }
        break;

        case actionGoToR:
        {
          LinkGoToR * g = (LinkGoToR *) a;
          // copy link file
          const char * fileName = g->getFileName()->getCString();
          // ceate link: fileName, namedDest, object pointer
          popplerLink = new LinkGoto( linkArea, (QString)fileName, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), m_page->parentDoc->m_doc ) ) );
        }
        break;

        case actionLaunch:
          LinkLaunch * e = (LinkLaunch *)a;
          GooString * p = e->getParams();
          popplerLink = new LinkExecute( linkArea, e->getFileName()->getCString(), p ? p->getCString() : 0 );
        break;

        case actionNamed:
          const char * name = ((LinkNamed *)a)->getName()->getCString();
          if ( !strcmp( name, "NextPage" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::PageNext );
          else if ( !strcmp( name, "PrevPage" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::PagePrev );
          else if ( !strcmp( name, "FirstPage" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::PageFirst );
          else if ( !strcmp( name, "LastPage" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::PageLast );
          else if ( !strcmp( name, "GoBack" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::HistoryBack );
          else if ( !strcmp( name, "GoForward" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::HistoryForward );
          else if ( !strcmp( name, "Quit" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::Quit );
          else if ( !strcmp( name, "GoToPage" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::GoToPage );
          else if ( !strcmp( name, "Find" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::Find );
          else if ( !strcmp( name, "FullScreen" ) )
              popplerLink = new LinkAction( linkArea, LinkAction::Presentation );
          else if ( !strcmp( name, "Close" ) )
          {
              // acroread closes the document always, doesnt care whether 
              // its presentation mode or not
              // popplerLink = new LinkAction( linkArea, LinkAction::EndPresentation );
              popplerLink = new LinkAction( linkArea, LinkAction::Close );
          }
          else
          {
                // TODO
          }
        break;

        case actionURI:
          popplerLink = new LinkBrowse( linkArea, ((LinkURI *)a)->getURI()->getCString() );
        break;

        case actionMovie:
/*      TODO this (Movie link)
          m_type = Movie;
          LinkMovie * m = (LinkMovie *) a;
          // copy Movie parameters (2 IDs and a const char *)
          Ref * r = m->getAnnotRef();
          m_refNum = r->num;
          m_refGen = r->gen;
          copyString( m_uri, m->getTitle()->getCString() );
*/      break;

        case actionUnknown:
        break;
      }
    }
    
    if (popplerLink)
    {
      popplerLinks.append(popplerLink);
    }
  }

  delete xpdfLinks;
  
  return popplerLinks;
}

}
