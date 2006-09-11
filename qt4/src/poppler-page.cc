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
#include "poppler-annotation-helper.h"

namespace Poppler {

class PageData {
  public:
  Link* convertLinkActionToLink(::LinkAction * a, const QRectF &linkArea, DocumentData * doc);

  const Document *parentDoc;
  int index;
  PageTransition *transition;
};

Link* PageData::convertLinkActionToLink(::LinkAction * a, const QRectF &linkArea, DocumentData * doc)
{
  if ( !a )
    return NULL;

  Link * popplerLink = NULL;
  switch ( a->getKind() )
  {
    case actionGoTo:
    {
      LinkGoTo * g = (LinkGoTo *) a;
      // create link: no ext file, namedDest, object pointer
      popplerLink = new LinkGoto( linkArea, QString::null, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), doc ) ) );
    }
    break;

    case actionGoToR:
    {
      LinkGoToR * g = (LinkGoToR *) a;
      // copy link file
      const char * fileName = g->getFileName()->getCString();
      // ceate link: fileName, namedDest, object pointer
      popplerLink = new LinkGoto( linkArea, (QString)fileName, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), doc ) ) );
    }
    break;

    case actionLaunch:
    {
      LinkLaunch * e = (LinkLaunch *)a;
      GooString * p = e->getParams();
      popplerLink = new LinkExecute( linkArea, e->getFileName()->getCString(), p ? p->getCString() : 0 );
    }
    break;

    case actionNamed:
    {
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
    }
    break;

    case actionURI:
    {
      popplerLink = new LinkBrowse( linkArea, ((LinkURI *)a)->getURI()->getCString() );
    }
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
*/  break;

    case actionUnknown:
    break;
  }

  return popplerLink;
}


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

QImage Page::splashRenderToImage(double xres, double yres, int x, int y, int w, int h, bool doLinks, Rotation rotate) const
{
  SplashOutputDev *output_dev = m_page->parentDoc->m_doc->getSplashOutputDev();
  
  int rotation = (int)rotate * 90;
  
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, xres, yres,
						 rotation, false, true, doLinks, x, y, w, h);
  
  SplashBitmap *bitmap = output_dev->getBitmap ();
  int bw = bitmap->getWidth();
  int bh = bitmap->getHeight();
  
  SplashColorPtr dataPtr = output_dev->getBitmap()->getDataPtr();
  
  if (QSysInfo::BigEndian == QSysInfo::ByteOrder)
  {
    uchar c;
    int count = bw * bh * 4;
    for (int k = 0; k < count; k += 4)
    {
      c = dataPtr[k];
      dataPtr[k] = dataPtr[k+3];
      dataPtr[k+3] = c;

      c = dataPtr[k+1];
      dataPtr[k+1] = dataPtr[k+2];
      dataPtr[k+2] = c;
    }
  }
  
  // construct a qimage SHARING the raw bitmap data in memory
  QImage img( dataPtr, bw, bh, QImage::Format_ARGB32 );
  img = img.copy();
  // unload underlying xpdf bitmap
  output_dev->startPage( 0, NULL );

  return img;
}

QPixmap *Page::splashRenderToPixmap(double xres, double yres, int x, int y, int w, int h, bool doLinks, Rotation rotate) const
{
  QImage img = splashRenderToImage(xres, yres, x, y, w, h, doLinks, rotate);

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

bool Page::search(const QString &text, QRectF &rect, SearchDirection direction, SearchMode caseSensitive, Rotation rotate) const
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

  int rotation = (int)rotate * 90;

  // fetch ourselves a textpage
  TextOutputDev td(NULL, gTrue, gFalse, gFalse);
  m_page->parentDoc->m_doc->doc.displayPage( &td, m_page->index + 1, 72, 72, rotation, false, true, false );
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

QList<TextBox*> Page::textList(Rotation rotate) const
{
  TextOutputDev *output_dev;
  
  QList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  
  int rotation = (int)rotate * 90;

  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      rotation, false, false, false, -1, -1, -1, -1);

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

    Link *popplerLink = m_page->convertLinkActionToLink(xpdfLink->getAction(), linkArea, m_page->parentDoc->m_doc);
    if (popplerLink)
    {
      popplerLinks.append(popplerLink);
    }
  }

  delete xpdfLinks;
  
  return popplerLinks;
}

QList<Annotation*> Page::annotations() const
{
    Object annotArray;
    ::Page *pdfPage = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
    pdfPage->getAnnots( &annotArray );
    if ( !annotArray.isArray() || annotArray.arrayGetLength() < 1 )
        return QList<Annotation*>();

    // ID to Annotation/PopupWindow maps
    QMap< int, Annotation * > annotationsMap;
    QMap< int, PopupWindow * > popupsMap;
    // lists of Windows and Revisions that needs resolution
    QLinkedList< ResolveRevision > resolveRevList;
    QLinkedList< ResolveWindow > resolvePopList;
    QLinkedList< PostProcessText > ppTextList;

    // build a normalized transform matrix for this page at 100% scale
    GfxState * gfxState = new GfxState( 72.0, 72.0, pdfPage->getMediaBox(), pdfPage->getRotate(), gTrue );
    double * gfxCTM = gfxState->getCTM();
    double MTX[6];
    for ( int i = 0; i < 6; i+=2 )
    {
        MTX[i] = gfxCTM[i] / pdfPage->getCropWidth();
        MTX[i+1] = gfxCTM[i+1] / pdfPage->getCropHeight();
    }
    delete gfxState;

    /** 1 - PARSE ALL ANNOTATIONS AND POPUPS FROM THE PAGE */
    Object annot;
    Object annotRef;    // no need to free this (impl. dependent!)
    uint numAnnotations = annotArray.arrayGetLength();
    for ( uint j = 0; j < numAnnotations; j++ )
    {
        // get the j-th annotation
        annotArray.arrayGet( j, &annot );
        if ( !annot.isDict() )
        {
            qDebug() << "PDFGenerator: annot not dictionary." << endl;
            annot.free();
            continue;
        }

        Annotation * annotation = 0;
        Dict * annotDict = annot.getDict();
        int annotID = annotArray.arrayGetNF( j, &annotRef )->getRefNum();
        bool parseMarkup = true,    // nearly all supported annots are markup
             addToPage = true;      // Popup annots are added to custom queue

        /** 1.1. GET Subtype */
        QString subType;
        XPDFReader::lookupName( annotDict, "Subtype", subType );
        if ( subType.isEmpty() )
        {
            qDebug() << "annot has no Subtype" << endl;
            annot.free();
            continue;
        }

        /** 1.2. CREATE Annotation / PopupWindow and PARSE specific params */
        if ( subType == "Text" || subType == "FreeText" )
        {
            // parse TextAnnotation params
            TextAnnotation * t = new TextAnnotation();
            annotation = t;

            if ( subType == "Text" )
            {
                // -> textType
                t->textType = TextAnnotation::Linked;
                // -> textIcon
                XPDFReader::lookupName( annotDict, "Name", t->textIcon );
                if ( !t->textIcon.isEmpty() )
                {
                    t->textIcon = t->textIcon.toLower();
                    t->textIcon.remove( ' ' );
                }
                // request for postprocessing window geometry
                PostProcessText request;
                request.textAnnotation = t;
                request.opened = false;
                XPDFReader::lookupBool( annotDict, "Open", request.opened );
                ppTextList.append( request );
            }
            else
            {
                // NOTE: please provide testcases for FreeText (don't have any) - Enrico
                // -> textType
                t->textType = TextAnnotation::InPlace;
                // -> textFont
                QString textFormat;
                XPDFReader::lookupString( annotDict, "DA", textFormat );
                // TODO, fill t->textFont using textFormat if not empty
                // -> inplaceAlign
                XPDFReader::lookupInt( annotDict, "Q", t->inplaceAlign );
                // -> inplaceText (simple)
                XPDFReader::lookupString( annotDict, "DS", t->inplaceText );
                // -> inplaceText (complex override)
                XPDFReader::lookupString( annotDict, "RC", t->inplaceText );
                // -> inplaceCallout
                double c[6];
                int n = XPDFReader::lookupNumArray( annotDict, "CL", c, 6 );
                if ( n >= 4 )
                {
                    XPDFReader::transform( MTX, c[0], c[1], t->inplaceCallout[0] );
                    XPDFReader::transform( MTX, c[2], c[3], t->inplaceCallout[1] );
                    if ( n == 6 )
                        XPDFReader::transform( MTX, c[4], c[5], t->inplaceCallout[2] );
                }
                // -> inplaceIntent
                QString intentName;
                XPDFReader::lookupString( annotDict, "IT", intentName );
                if ( intentName == "FreeTextCallout" )
                    t->inplaceIntent = TextAnnotation::Callout;
                else if ( intentName == "FreeTextTypeWriter" )
                    t->inplaceIntent = TextAnnotation::TypeWriter;
            }
        }
        else if ( subType == "Line" || subType == "Polygon" || subType == "PolyLine" )
        {
            // parse LineAnnotation params
            LineAnnotation * l = new LineAnnotation();
            annotation = l;

            // -> linePoints
            double c[100];
            int num = XPDFReader::lookupNumArray( annotDict, (subType == "Line") ? "L" : "Vertices", c, 100 );
            if ( num < 4 || (num % 2) != 0 )
            {
                qDebug() << "L/Vertices wrong fol Line/Poly." << endl;
                delete annotation;
                annot.free();
                continue;
            }
            for ( int i = 0; i < num; i += 2 )
            {
                QPointF p;
                XPDFReader::transform( MTX, c[i], c[i+1], p );
                l->linePoints.push_back( p );
            }
            // -> lineStartStyle, lineEndStyle
            Object leArray;
            annotDict->lookup( "LE", &leArray );
            if ( leArray.isArray() && leArray.arrayGetLength() == 2 )
            {
                // -> lineStartStyle
                Object styleObj;
                leArray.arrayGet( 0, &styleObj );
                if ( styleObj.isName() )
                {
                    const char * name = styleObj.getName();
                    if ( !strcmp( name, "Square" ) )
                        l->lineStartStyle = LineAnnotation::Square;
                    else if ( !strcmp( name, "Circle" ) )
                        l->lineStartStyle = LineAnnotation::Circle;
                    else if ( !strcmp( name, "Diamond" ) )
                        l->lineStartStyle = LineAnnotation::Diamond;
                    else if ( !strcmp( name, "OpenArrow" ) )
                        l->lineStartStyle = LineAnnotation::OpenArrow;
                    else if ( !strcmp( name, "ClosedArrow" ) )
                        l->lineStartStyle = LineAnnotation::ClosedArrow;
                    else if ( !strcmp( name, "None" ) )
                        l->lineStartStyle = LineAnnotation::None;
                    else if ( !strcmp( name, "Butt" ) )
                        l->lineStartStyle = LineAnnotation::Butt;
                    else if ( !strcmp( name, "ROpenArrow" ) )
                        l->lineStartStyle = LineAnnotation::ROpenArrow;
                    else if ( !strcmp( name, "RClosedArrow" ) )
                        l->lineStartStyle = LineAnnotation::RClosedArrow;
                    else if ( !strcmp( name, "Slash" ) )
                        l->lineStartStyle = LineAnnotation::Slash;
                }
                styleObj.free();
                // -> lineEndStyle
                leArray.arrayGet( 1, &styleObj );
                if ( styleObj.isName() )
                {
                    const char * name = styleObj.getName();
                    if ( !strcmp( name, "Square" ) )
                        l->lineEndStyle = LineAnnotation::Square;
                    else if ( !strcmp( name, "Circle" ) )
                        l->lineEndStyle = LineAnnotation::Circle;
                    else if ( !strcmp( name, "Diamond" ) )
                        l->lineEndStyle = LineAnnotation::Diamond;
                    else if ( !strcmp( name, "OpenArrow" ) )
                        l->lineEndStyle = LineAnnotation::OpenArrow;
                    else if ( !strcmp( name, "ClosedArrow" ) )
                        l->lineEndStyle = LineAnnotation::ClosedArrow;
                    else if ( !strcmp( name, "None" ) )
                        l->lineEndStyle = LineAnnotation::None;
                    else if ( !strcmp( name, "Butt" ) )
                        l->lineEndStyle = LineAnnotation::Butt;
                    else if ( !strcmp( name, "ROpenArrow" ) )
                        l->lineEndStyle = LineAnnotation::ROpenArrow;
                    else if ( !strcmp( name, "RClosedArrow" ) )
                        l->lineEndStyle = LineAnnotation::RClosedArrow;
                    else if ( !strcmp( name, "Slash" ) )
                        l->lineEndStyle = LineAnnotation::Slash;
                }
                styleObj.free();
            }
            leArray.free();
            // -> lineClosed
            l->lineClosed = subType == "Polygon";
            // -> lineInnerColor
            XPDFReader::lookupColor( annotDict, "IC", l->lineInnerColor );
            // -> lineLeadingFwdPt
            XPDFReader::lookupNum( annotDict, "LL", l->lineLeadingFwdPt );
            // -> lineLeadingBackPt
            XPDFReader::lookupNum( annotDict, "LLE", l->lineLeadingBackPt );
            // -> lineShowCaption
            XPDFReader::lookupBool( annotDict, "Cap", l->lineShowCaption );
            // -> lineIntent
            QString intentName;
            XPDFReader::lookupString( annotDict, "IT", intentName );
            if ( intentName == "LineArrow" )
                l->lineIntent = LineAnnotation::Arrow;
            else if ( intentName == "LineDimension" )
                l->lineIntent = LineAnnotation::Dimension;
            else if ( intentName == "PolygonCloud" )
                l->lineIntent = LineAnnotation::PolygonCloud;
        }
        else if ( subType == "Square" || subType == "Circle" )
        {
            // parse GeomAnnotation params
            GeomAnnotation * g = new GeomAnnotation();
            annotation = g;

            // -> geomType
            if ( subType == "Square" )
                g->geomType = GeomAnnotation::InscribedSquare;
            else
                g->geomType = GeomAnnotation::InscribedCircle;
            // -> geomInnerColor
            XPDFReader::lookupColor( annotDict, "IC", g->geomInnerColor );
            // TODO RD
        }
        else if ( subType == "Highlight" || subType == "Underline" ||
                  subType == "Squiggly" || subType == "StrikeOut" )
        {
            // parse HighlightAnnotation params
            HighlightAnnotation * h = new HighlightAnnotation();
            annotation = h;

            // -> highlightType
            if ( subType == "Highlight" )
                h->highlightType = HighlightAnnotation::Highlight;
            else if ( subType == "Underline" )
                h->highlightType = HighlightAnnotation::Underline;
            else if ( subType == "Squiggly" )
                h->highlightType = HighlightAnnotation::Squiggly;
            else if ( subType == "StrikeOut" )
                h->highlightType = HighlightAnnotation::StrikeOut;

            // -> highlightQuads
            double c[80];
            int num = XPDFReader::lookupNumArray( annotDict, "QuadPoints", c, 80 );
            if ( num < 8 || (num % 8) != 0 )
            {
                qDebug() << "Wrong QuadPoints for a Highlight annotation." << endl;
                delete annotation;
                annot.free();
                continue;
            }
            for ( int q = 0; q < num; q += 8 )
            {
                HighlightAnnotation::Quad quad;
                for ( int p = 0; p < 4; p++ )
                    XPDFReader::transform( MTX, c[ q + p*2 ], c[ q + p*2 + 1 ], quad.points[ p ] );
                // ### PDF1.6 specs says that point are in ccw order, but in fact
                // points 3 and 4 are swapped in every PDF around!
                QPointF tmpPoint = quad.points[ 2 ];
                quad.points[ 2 ] = quad.points[ 3 ];
                quad.points[ 3 ] = tmpPoint;
                // initialize other oroperties and append quad
                quad.capStart = true;       // unlinked quads are always capped
                quad.capEnd = true;         // unlinked quads are always capped
                quad.feather = 0.1;         // default feather
                h->highlightQuads.append( quad );
            }
        }
        else if ( subType == "Stamp" )
        {
            // parse StampAnnotation params
            StampAnnotation * s = new StampAnnotation();
            annotation = s;

            // -> stampIconName
            XPDFReader::lookupName( annotDict, "Name", s->stampIconName );
        }
        else if ( subType == "Ink" )
        {
            // parse InkAnnotation params
            InkAnnotation * k = new InkAnnotation();
            annotation = k;

            // -> inkPaths
            Object pathsArray;
            annotDict->lookup( "InkList", &pathsArray );
            if ( !pathsArray.isArray() || pathsArray.arrayGetLength() < 1 )
            {
                qDebug() << "InkList not present for ink annot" << endl;
                delete annotation;
                annot.free();
                continue;
            }
            int pathsNumber = pathsArray.arrayGetLength();
            for ( int m = 0; m < pathsNumber; m++ )
            {
                // transform each path in a list of normalized points ..
                QLinkedList<QPointF> localList;
                Object pointsArray;
                pathsArray.arrayGet( m, &pointsArray );
                if ( pointsArray.isArray() )
                {
                    int pointsNumber = pointsArray.arrayGetLength();
                    for ( int n = 0; n < pointsNumber; n+=2 )
                    {
                        // get the x,y numbers for current point
                        Object numObj;
                        double x = pointsArray.arrayGet( n, &numObj )->getNum();
                        numObj.free();
                        double y = pointsArray.arrayGet( n+1, &numObj )->getNum();
                        numObj.free();
                        // add normalized point to localList
                        QPointF np;
                        XPDFReader::transform( MTX, x, y, np );
                        localList.push_back( np );
                    }
                }
                pointsArray.free();
                // ..and add it to the annotation
                k->inkPaths.push_back( localList );
            }
            pathsArray.free();
        }
        else if ( subType == "Popup" )
        {
            // create PopupWindow and add it to the popupsMap
            PopupWindow * popup = new PopupWindow();
            popupsMap[ annotID ] = popup;
            parseMarkup = false;
            addToPage = false;

            // get window specific properties if any
            popup->shown = false;
            XPDFReader::lookupBool( annotDict, "Open", popup->shown );
            // no need to parse parent annotation id
            //XPDFReader::lookupIntRef( annotDict, "Parent", popup->... );

            // use the 'dummy annotation' for getting other parameters
            popup->dummyAnnotation = new Annotation();
            annotation = popup->dummyAnnotation;
        }
        else if ( subType == "Link" )
        {
            // parse Link params
            LinkAnnotation * l = new LinkAnnotation();
            annotation = l;

            // -> hlMode
            QString hlModeString;
            XPDFReader::lookupString( annotDict, "H", hlModeString );
            if ( hlModeString == "N" )
                l->linkHLMode = LinkAnnotation::None;
            else if ( hlModeString == "I" )
                l->linkHLMode = LinkAnnotation::Invert;
            else if ( hlModeString == "O" )
                l->linkHLMode = LinkAnnotation::Outline;
            else if ( hlModeString == "P" )
                l->linkHLMode = LinkAnnotation::Push;

            // -> link region
            double c[8];
            int num = XPDFReader::lookupNumArray( annotDict, "QuadPoints", c, 8 );
            if ( num > 0 && num != 8 )
            {
                qDebug() << "Wrong QuadPoints for a Link annotation." << endl;
                delete annotation;
                annot.free();
                continue;
            }
            if ( num == 8 )
            {
                XPDFReader::transform( MTX, c[ 0 ], c[ 1 ], l->linkRegion[ 0 ] );
                XPDFReader::transform( MTX, c[ 2 ], c[ 3 ], l->linkRegion[ 1 ] );
                XPDFReader::transform( MTX, c[ 4 ], c[ 5 ], l->linkRegion[ 2 ] );
                XPDFReader::transform( MTX, c[ 6 ], c[ 7 ], l->linkRegion[ 3 ] );
            }

            // reading link action
            Object objPA;
            annotDict->lookup( "PA", &objPA );
            ::LinkAction * a = ::LinkAction::parseAction( &objPA, m_page->parentDoc->m_doc->doc.getCatalog()->getBaseURI() );
            Link * popplerLink = m_page->convertLinkActionToLink( a, QRectF(), m_page->parentDoc->m_doc );
            if ( popplerLink )
            {
                l->linkDestination = popplerLink;
            }
            objPA.free();
        }
        else
        {
            // MISSING: Caret, FileAttachment, Sound, Movie, Widget,
            //          Screen, PrinterMark, TrapNet, Watermark, 3D
            qDebug() << "annotation '" << subType << "' not supported" << endl;
            annot.free();
            continue;
        }

        /** 1.3. PARSE common parameters */
        // -> boundary
        double r[4];
        if ( XPDFReader::lookupNumArray( annotDict, "Rect", r, 4 ) != 4 )
        {
            qDebug() << "Rect is missing for annotation." << endl;
            annot.free();
            continue;
        }
        // transform annotation rect to uniform coords
        QPointF topLeft, bottomRight;
        XPDFReader::transform( MTX, r[0], r[1], topLeft );
        XPDFReader::transform( MTX, r[2], r[3], bottomRight );
        annotation->boundary.setTopLeft(topLeft);
        annotation->boundary.setBottomRight(bottomRight);
        if ( annotation->boundary.left() > annotation->boundary.right() )
        {
            double aux = annotation->boundary.left();
            annotation->boundary.setLeft(annotation->boundary.right());
            annotation->boundary.setRight(aux);
        }
        if ( annotation->boundary.top() > annotation->boundary.bottom() )
        {
            double aux = annotation->boundary.top();
            annotation->boundary.setTop(annotation->boundary.bottom());
            annotation->boundary.setBottom(aux);
           //annotation->rUnscaledWidth = (r[2] > r[0]) ? r[2] - r[0] : r[0] - r[2];
           //annotation->rUnscaledHeight = (r[3] > r[1]) ? r[3] - r[1] : r[1] - r[3];
        }
        // -> contents
        XPDFReader::lookupString( annotDict, "Contents", annotation->contents );
        // -> uniqueName
        XPDFReader::lookupString( annotDict, "NM", annotation->uniqueName );
        // -> modifyDate (and -> creationDate)
        XPDFReader::lookupDate( annotDict, "M", annotation->modifyDate );
        if ( annotation->creationDate.isNull() && !annotation->modifyDate.isNull() )
            annotation->creationDate = annotation->modifyDate;
        // -> flags: set the external attribute since it's embedded on file
        annotation->flags |= Annotation::External;
        // -> flags
        int flags = 0;
        XPDFReader::lookupInt( annotDict, "F", flags );
        if ( flags & 0x2 )
            annotation->flags |= Annotation::Hidden;
        if ( flags & 0x8 )
            annotation->flags |= Annotation::FixedSize;
        if ( flags & 0x10 )
            annotation->flags |= Annotation::FixedRotation;
        if ( !(flags & 0x4) )
            annotation->flags |= Annotation::DenyPrint;
        if ( flags & 0x40 )
            annotation->flags |= (Annotation::DenyWrite | Annotation::DenyDelete);
        if ( flags & 0x80 )
            annotation->flags |= Annotation::DenyDelete;
        if ( flags & 0x100 )
            annotation->flags |= Annotation::ToggleHidingOnMouse;
        // -> style (Border(old spec), BS, BE)
        double border[3];
        int bn = XPDFReader::lookupNumArray( annotDict, "Border", border, 3 );
        if ( bn == 3 )
        {
            // -> style.xCorners
            annotation->style.xCorners = border[0];
            // -> style.yCorners
            annotation->style.yCorners = border[1];
            // -> style.width
            annotation->style.width = border[2];
        }
        Object bsObj;
        annotDict->lookup( "BS", &bsObj );
        if ( bsObj.isDict() )
        {
            // -> style.width
            XPDFReader::lookupNum( bsObj.getDict(), "W", annotation->style.width );
            // -> style.style
            QString styleName;
            XPDFReader::lookupName( bsObj.getDict(), "S", styleName );
            if ( styleName == "S" )
                annotation->style.style = Annotation::Solid;
            else if ( styleName == "D" )
                annotation->style.style = Annotation::Dashed;
            else if ( styleName == "B" )
                annotation->style.style = Annotation::Beveled;
            else if ( styleName == "I" )
                annotation->style.style = Annotation::Inset;
            else if ( styleName == "U" )
                annotation->style.style = Annotation::Underline;
            // -> style.marks and style.spaces
            Object dashArray;
            bsObj.getDict()->lookup( "D", &dashArray );
            if ( dashArray.isArray() )
            {
                int dashMarks = 3;
                int dashSpaces = 0;
                Object intObj;
                dashArray.arrayGet( 0, &intObj );
                if ( intObj.isInt() )
                    dashMarks = intObj.getInt();
                intObj.free();
                dashArray.arrayGet( 1, &intObj );
                if ( intObj.isInt() )
                    dashSpaces = intObj.getInt();
                intObj.free();
                annotation->style.marks = dashMarks;
                annotation->style.spaces = dashSpaces;
            }
            dashArray.free();
        }
        bsObj.free();
        Object beObj;
        annotDict->lookup( "BE", &beObj );
        if ( beObj.isDict() )
        {
            // -> style.effect
            QString effectName;
            XPDFReader::lookupName( beObj.getDict(), "S", effectName );
            if ( effectName == "C" )
                annotation->style.effect = Annotation::Cloudy;
            // -> style.effectIntensity
            int intensityInt = -1;
            XPDFReader::lookupInt( beObj.getDict(), "I", intensityInt );
            if ( intensityInt != -1 )
                annotation->style.effectIntensity = (double)intensityInt;
        }
        beObj.free();
        // -> style.color
        XPDFReader::lookupColor( annotDict, "C", annotation->style.color );

        /** 1.4. PARSE markup { common, Popup, Revision } parameters */
        if ( parseMarkup )
        {
            // -> creationDate
            XPDFReader::lookupDate( annotDict, "CreationDate", annotation->creationDate );
            // -> style.opacity
            XPDFReader::lookupNum( annotDict, "CA", annotation->style.opacity );
            // -> window.title and author
            XPDFReader::lookupString( annotDict, "T", annotation->window.title );
            annotation->author = annotation->window.title;
            // -> window.summary
            XPDFReader::lookupString( annotDict, "Subj", annotation->window.summary );
            // -> window.text
            XPDFReader::lookupString( annotDict, "RC", annotation->window.text );

            // if a popup is referenced, schedule for resolving it later
            int popupID = 0;
            XPDFReader::lookupIntRef( annotDict, "Popup", popupID );
            if ( popupID )
            {
                ResolveWindow request;
                request.popupWindowID = popupID;
                request.annotation = annotation;
                resolvePopList.append( request );
            }

            // if an older version is referenced, schedule for reparenting
            int parentID = 0;
            XPDFReader::lookupIntRef( annotDict, "IRT", parentID );
            if ( parentID )
            {
                ResolveRevision request;
                request.nextAnnotation = annotation;
                request.nextAnnotationID = annotID;
                request.prevAnnotationID = parentID;

                // -> request.nextScope
                request.nextScope = Annotation::Reply;
                Object revObj;
                annotDict->lookup( "RT", &revObj );
                if ( revObj.isName() )
                {
                    const char * name = revObj.getName();
                    if ( !strcmp( name, "R" ) )
                        request.nextScope = Annotation::Reply;
                    else if ( !strcmp( name, "Group" ) )
                        request.nextScope = Annotation::Group;
                }
                revObj.free();

                // -> revision.type (StateModel is deduced from type, not parsed)
                request.nextType = Annotation::None;
                annotDict->lookup( "State", &revObj );
                if ( revObj.isString() )
                {
                    const char * name = revObj.getString()->getCString();
                    if ( !strcmp( name, "Marked" ) )
                        request.nextType = Annotation::Marked;
                    else if ( !strcmp( name, "Unmarked" ) )
                        request.nextType = Annotation::Unmarked;
                    else if ( !strcmp( name, "Accepted" ) )
                        request.nextType = Annotation::Accepted;
                    else if ( !strcmp( name, "Rejected" ) )
                        request.nextType = Annotation::Rejected;
                    else if ( !strcmp( name, "Cancelled" ) )
                        request.nextType = Annotation::Cancelled;
                    else if ( !strcmp( name, "Completed" ) )
                        request.nextType = Annotation::Completed;
                    else if ( !strcmp( name, "None" ) )
                        request.nextType = Annotation::None;
                }
                revObj.free();

                // schedule for later reparenting
                resolveRevList.append( request );
            }
        }
        // free annot object
        annot.free();

        /** 1.5. ADD ANNOTATION to the annotationsMap  */
        if ( addToPage )
        {
            if ( annotationsMap.contains( annotID ) )
                qDebug() << "PDFGenerator: clash for annotations with ID:" << annotID << endl;
            annotationsMap[ annotID ] = annotation;
        }
    } // end Annotation/PopupWindow parsing loop

    /** 2 - RESOLVE POPUPS (popup.* -> annotation.window) */
    if ( !resolvePopList.isEmpty() && !popupsMap.isEmpty() )
    {
        QLinkedList< ResolveWindow >::iterator it = resolvePopList.begin(),
                                              end = resolvePopList.end();
        for ( ; it != end; ++it )
        {
            const ResolveWindow & request = *it;
            if ( !popupsMap.contains( request.popupWindowID ) )
                // warn aboud problems in popup resolving logic
                qDebug() << "PDFGenerator: can't resolve popup "
                          << request.popupWindowID << "." << endl;
            else
            {
                // set annotation's window properties taking ones from popup
                PopupWindow * pop = popupsMap[ request.popupWindowID ];
                Annotation * pa = pop->dummyAnnotation;
                Annotation::Window & w = request.annotation->window;

                // transfer properties to Annotation's window
                w.flags = pa->flags & (Annotation::Hidden |
                    Annotation::FixedSize | Annotation::FixedRotation);
                if ( !pop->shown )
                    w.flags |= Annotation::Hidden;
                w.topLeft.setX(pa->boundary.left());
                w.topLeft.setY(pa->boundary.top());
                w.width = (int)( pa->boundary.right() - pa->boundary.left() );
                w.height = (int)( pa->boundary.bottom() - pa->boundary.top() );
            }
        }

        // clear data
        QMap< int, PopupWindow * >::Iterator dIt = popupsMap.begin(), dEnd = popupsMap.end();
        for ( ; dIt != dEnd; ++dIt )
        {
            PopupWindow * p = dIt.value();
            delete p->dummyAnnotation;
            delete p;
        }
    }

    /** 3 - RESOLVE REVISIONS (parent.revisions.append( children )) */
    if ( !resolveRevList.isEmpty() )
    {
        // append children to parents
        int excludeIDs[ resolveRevList.count() ];   // can't even reach this size
        int excludeIndex = 0;                       // index in excludeIDs array
        QLinkedList< ResolveRevision >::iterator it = resolveRevList.begin(), end = resolveRevList.end();
        for ( ; it != end; ++it )
        {
            const ResolveRevision & request = *it;
            int parentID = request.prevAnnotationID;
            if ( !annotationsMap.contains( parentID ) )
                // warn about problems in reparenting logic
                qDebug() << "PDFGenerator: can't reparent annotation to "
                          << parentID << "." << endl;
            else
            {
                // compile and add a Revision structure to the parent annotation
                Annotation::Revision childRevision;
                childRevision.annotation = request.nextAnnotation;
                childRevision.scope = request.nextScope;
                childRevision.type = request.nextType;
                annotationsMap[ parentID ]->revisions.append( childRevision );
                // exclude child annotation from being rooted in page
                excludeIDs[ excludeIndex++ ] = request.nextAnnotationID;
            }
        }

        // prevent children from being attached to page as roots
        for ( int i = 0; i < excludeIndex; i++ )
            annotationsMap.remove( excludeIDs[ i ] );
    }

    /** 4 - POSTPROCESS TextAnnotations (when window geom is embedded) */
    if ( !ppTextList.isEmpty() )
    {
        QLinkedList< PostProcessText >::const_iterator it = ppTextList.begin(), end = ppTextList.end();
        for ( ; it != end; ++it )
        {
            const PostProcessText & request = *it;
            Annotation::Window & window = request.textAnnotation->window;
            // if not present, 'create' the window in-place over the annotation
            if ( window.flags == -1 )
            {
                window.flags = 0;
                QRectF & geom = request.textAnnotation->boundary;
                // initialize window geometry to annotation's one
                window.width = (int)( geom.right() - geom.left() );
                window.height = (int)( geom.bottom() - geom.top() );
                window.topLeft.setX( geom.left() > 0.0 ? geom.left() : 0.0 );
                window.topLeft.setY( geom.top() > 0.0 ? geom.top() : 0.0 );
            }
            // (pdf) if text is not 'opened', force window hiding. if the window
            // was parsed from popup, the flag should already be set
            if ( !request.opened && window.flags != -1 )
                window.flags |= Annotation::Hidden;
        }
    }

    /** 5 - finally RETURN ANNOTATIONS */
    return annotationsMap.values();
}


}
