//========================================================================
//
// ArthurOutputDev.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include <math.h>

#include "goo/gfile.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Link.h"
#include "CharCodeToUnicode.h"
#include "FontEncodingTables.h"
#include <fofi/FoFiTrueType.h>
#include "ArthurOutputDev.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainterPath>
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// ArthurOutputDev
//------------------------------------------------------------------------

ArthurOutputDev::ArthurOutputDev(QPainter *painter):
  m_painter(painter)
{
  m_currentBrush = QBrush(Qt::SolidPattern);
}

ArthurOutputDev::~ArthurOutputDev()
{
}

void ArthurOutputDev::startDoc(XRef *xrefA) {
}

void ArthurOutputDev::startPage(int pageNum, GfxState *state)
{
}

void ArthurOutputDev::endPage() {
}

void ArthurOutputDev::drawLink(Link *link, Catalog *catalog)
{
}

void ArthurOutputDev::saveState(GfxState *state)
{
  m_painter->save();
}

void ArthurOutputDev::restoreState(GfxState *state)
{
  m_painter->restore();
}

void ArthurOutputDev::updateAll(GfxState *state)
{
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
  updateFlatness(state);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  updateFillOpacity(state);
  updateStrokeOpacity(state);
  m_needFontUpdate = gTrue;
}

// This looks wrong - why aren't adjusting the matrix?
void ArthurOutputDev::updateCTM(GfxState *state, double m11, double m12,
				double m21, double m22,
				double m31, double m32)
{
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
}

void ArthurOutputDev::updateLineDash(GfxState *state)
{
  // qDebug() << "updateLineDash";
}

void ArthurOutputDev::updateFlatness(GfxState *state)
{
  // qDebug() << "updateFlatness";
}

void ArthurOutputDev::updateLineJoin(GfxState *state)
{
  switch (state->getLineJoin()) {
  case 0:
    m_currentPen.setJoinStyle(Qt::MiterJoin);
    break;
  case 1:
    m_currentPen.setJoinStyle(Qt::RoundJoin);
    break;
  case 2:
    m_currentPen.setJoinStyle(Qt::BevelJoin);
    break;
  }
  m_painter->setPen(m_currentPen);
}

void ArthurOutputDev::updateLineCap(GfxState *state)
{
  switch (state->getLineCap()) {
  case 0:
    m_currentPen.setCapStyle(Qt::FlatCap);
    break;
  case 1:
    m_currentPen.setCapStyle(Qt::RoundCap);
    break;
  case 2:
    m_currentPen.setCapStyle(Qt::SquareCap);
    break;
  }
  m_painter->setPen(m_currentPen);
}

void ArthurOutputDev::updateMiterLimit(GfxState *state)
{
  // We can't do mitre (or Miter) limit with Qt4 yet.
  // the limit is in state->getMiterLimit() when we get there
}

void ArthurOutputDev::updateLineWidth(GfxState *state)
{
  m_currentPen.setWidthF(state->getTransformedLineWidth());
  m_painter->setPen(m_currentPen);
}

void ArthurOutputDev::updateFillColor(GfxState *state)
{
  GfxRGB rgb;
  QColor brushColour = m_currentBrush.color();
  state->getFillRGB(&rgb);
  brushColour.setRgbF(rgb.r, rgb.g, rgb.b, brushColour.alphaF());
  m_currentBrush.setColor(brushColour);
  // TODO: why doesn't this work?
  // m_painter->setBrush(m_currentBrush);
}

void ArthurOutputDev::updateStrokeColor(GfxState *state)
{
  GfxRGB rgb;
  QColor penColour = m_currentPen.color();
  state->getStrokeRGB(&rgb);
  penColour.setRgbF(rgb.r, rgb.g, rgb.b, penColour.alphaF());
  m_currentPen.setColor(penColour);
  m_painter->setPen(m_currentPen);
}

void ArthurOutputDev::updateFillOpacity(GfxState *state)
{
  QColor brushColour= m_currentBrush.color();
  brushColour.setAlphaF(state->getFillOpacity());
  m_currentBrush.setColor(brushColour);
  // TODO: why doesn't this work?
  // m_painter->setBrush(m_currentBrush);
}

void ArthurOutputDev::updateStrokeOpacity(GfxState *state)
{
  QColor penColour= m_currentPen.color();
  penColour.setAlphaF(state->getStrokeOpacity());
  m_currentPen.setColor(penColour);
  m_painter->setPen(m_currentPen);
}

void ArthurOutputDev::updateFont(GfxState *state)
{
  // Something like
  // currentFont.setPointSize( state->getFontSize() );
  // m_painter->setFont(currentFont);
  // but with transformation matrices and such...
#if 0
  cairo_font_face_t *font_face;
  double m11, m12, m21, m22;
  double w;
  cairo_matrix_t matrix;

  LOG(printf ("updateFont() font=%s\n", state->getFont()->getName()->getCString()));
  
  /* Needs to be rethough, since fonts are now handled by cairo */
  m_needFontUpdate = gFalse;

  currentFont = fontEngine->getFont (state->getFont(), xref);

  state->getFontTransMat(&m11, &m12, &m21, &m22);
  m11 *= state->getHorizScaling();
  m12 *= state->getHorizScaling();

  w = currentFont->getSubstitutionCorrection(state->getFont());
  m12 *= w;
  m22 *= w;

  LOG(printf ("font matrix: %f %f %f %f\n", m11, m12, m21, m22));
  
  font_face = currentFont->getFontFace();
  cairo_set_font_face (cairo, font_face);

  matrix.xx = m11;
  matrix.xy = -m21;
  matrix.yx = m12;
  matrix.yy = -m22;
  matrix.x0 = 0;
  matrix.y0 = 0;
  cairo_set_font_matrix (cairo, &matrix);
#endif
}

static QPainterPath convertPath(GfxState *state, GfxPath *path, Qt::FillRule fillRule)
{
  GfxSubpath *subpath;
  double x1, y1, x2, y2, x3, y3;
  int i, j;

  QPainterPath qPath;
  qPath.setFillRule(fillRule);
  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    if (subpath->getNumPoints() > 0) {
      state->transform(subpath->getX(0), subpath->getY(0), &x1, &y1);
      qPath.moveTo(x1, y1);
      j = 1;
      while (j < subpath->getNumPoints()) {
	if (subpath->getCurve(j)) {
	  state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
	  state->transform(subpath->getX(j+1), subpath->getY(j+1), &x2, &y2);
	  state->transform(subpath->getX(j+2), subpath->getY(j+2), &x3, &y3);
	  qPath.cubicTo( x1, y1, x2, y2, x3, y3);
	  j += 3;
	} else {
	  state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
	  qPath.lineTo(x1, y1);
	  ++j;
	}
      }
      if (subpath->isClosed()) {
	qPath.closeSubpath();
      }
    }
  }
  return qPath;
}

void ArthurOutputDev::stroke(GfxState *state)
{
  m_painter->drawPath( convertPath( state, state->getPath(), Qt::OddEvenFill ) );
}

void ArthurOutputDev::fill(GfxState *state)
{
  m_painter->fillPath( convertPath( state, state->getPath(), Qt::WindingFill ), m_currentBrush );
}

void ArthurOutputDev::eoFill(GfxState *state)
{
  m_painter->fillPath( convertPath( state, state->getPath(), Qt::OddEvenFill ), m_currentBrush );
}

void ArthurOutputDev::clip(GfxState *state)
{
  m_painter->setClipPath(convertPath( state, state->getPath(), Qt::WindingFill ) );
}

void ArthurOutputDev::eoClip(GfxState *state)
{
  m_painter->setClipPath(convertPath( state, state->getPath(), Qt::OddEvenFill ) );
}

void ArthurOutputDev::drawString(GfxState *state, GooString *s)
{
  GfxFont *font;
  int wMode;
  int render;
  // the number of bytes in the string and not the number of glyphs?
  int len = s->getLength();
  char *p = s->getCString();
  int count = 0;
  double curX, curY;
  double riseX, riseY;

  font = state->getFont();
  wMode = font->getWMode();

  if (m_needFontUpdate) {
    updateFont(state);
  }

  // check for invisible text -- this is used by Acrobat Capture
  render = state->getRender();
  if (render == 3) {
    return;
  }

  // ignore empty strings
  if (len == 0)
    return;
  
  state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
  curX = state->getCurX();
  curY = state->getCurY();
  while (len > 0) {
    double x, y;
    double x1, y1;
    double dx, dy, tdx, tdy;
    double originX, originY, tOriginX, tOriginY;
    int n, uLen;
    CharCode code;
    Unicode u[8];
    n = font->getNextChar(p, len, &code,
	                  u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			  &dx, &dy, &originX, &originY);
    if (wMode) {
      dx *= state->getFontSize();
      dy = dy * state->getFontSize() + state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dy += state->getWordSpace();
      }
    } else {
      dx = dx * state->getFontSize() + state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dx += state->getWordSpace();
      }
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
    }
    originX *= state->getFontSize();
    originY *= state->getFontSize();
    state->textTransformDelta(dx, dy, &tdx, &tdy);
    state->textTransformDelta(originX, originY, &tOriginX, &tOriginY);
    x = curX + riseX;
    y = curY + riseY;
    x -= tOriginX;
    y -= tOriginY;
    state->transform(x, y, &x1, &y1);

    //glyphs[count].index = currentFont->getGlyph (code, u, uLen);
    m_painter->drawText(QPointF(x1,y1), QString(*p) );
    curX += tdx;
    curY += tdy;
    p += n;
    len -= n;
    count++;
  }
#if 0
  // fill
  if (!(render & 1)) {
    LOG (printf ("fill string\n"));
    cairo_set_source_rgb (cairo,
			 fill_color.r, fill_color.g, fill_color.b);
    cairo_show_glyphs (cairo, glyphs, count);
  }
  
  // stroke
  if ((render & 3) == 1 || (render & 3) == 2) {
    LOG (printf ("stroke string\n"));
    cairo_set_source_rgb (cairo,
			 stroke_color.r, stroke_color.g, stroke_color.b);
    cairo_glyph_path (cairo, glyphs, count);
    cairo_stroke (cairo);
  }

  // clip
  if (render & 4) {
    // FIXME: This is quite right yet, we need to accumulate all
    // glyphs within one text object before we clip.  Right now this
    // just add this one string.
    LOG (printf ("clip string\n"));
    cairo_glyph_path (cairo, glyphs, count);
    cairo_clip (cairo);
  }
#endif  
}

GBool ArthurOutputDev::beginType3Char(GfxState *state, double x, double y,
				      double dx, double dy,
				      CharCode code, Unicode *u, int uLen)
{
  return gFalse;
}

void ArthurOutputDev::endType3Char(GfxState *state)
{
}

void ArthurOutputDev::type3D0(GfxState *state, double wx, double wy)
{
}

void ArthurOutputDev::type3D1(GfxState *state, double wx, double wy,
			      double llx, double lly, double urx, double ury)
{
}

void ArthurOutputDev::endTextObject(GfxState *state)
{
}


void ArthurOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				    int width, int height, GBool invert,
				    GBool inlineImg)
{
  qDebug() << "drawImageMask";
#if 0
  unsigned char *buffer;
  unsigned char *dest;
  cairo_surface_t *image;
  cairo_pattern_t *pattern;
  int x, y;
  ImageStream *imgStr;
  Guchar *pix;
  double *ctm;
  cairo_matrix_t matrix;
  int invert_bit;
  int row_stride;

  row_stride = (width + 3) & ~3;
  buffer = (unsigned char *) malloc (height * row_stride);
  if (buffer == NULL) {
    error(-1, "Unable to allocate memory for image.");
    return;
  }

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width, 1, 1);
  imgStr->reset();

  invert_bit = invert ? 1 : 0;

  for (y = 0; y < height; y++) {
    pix = imgStr->getLine();
    dest = buffer + y * row_stride;
    for (x = 0; x < width; x++) {

      if (pix[x] ^ invert_bit)
	*dest++ = 0;
      else
	*dest++ = 255;
    }
  }

  image = cairo_image_surface_create_for_data (buffer, CAIRO_FORMAT_A8,
					  width, height, row_stride);
  if (image == NULL)
    return;
  pattern = cairo_pattern_create_for_surface (image);
  if (pattern == NULL)
    return;

  ctm = state->getCTM();
  LOG (printf ("drawImageMask %dx%d, matrix: %f, %f, %f, %f, %f, %f\n",
	       width, height, ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]));
  matrix.xx = ctm[0] / width;
  matrix.xy = -ctm[2] / height;
  matrix.yx = ctm[1] / width;
  matrix.yy = -ctm[3] / height;
  matrix.x0 = ctm[2] + ctm[4];
  matrix.y0 = ctm[3] + ctm[5];
  cairo_matrix_invert (&matrix);
  cairo_pattern_set_matrix (pattern, &matrix);

  cairo_pattern_set_filter (pattern, CAIRO_FILTER_BEST);
  /* FIXME: Doesn't the image mask support any colorspace? */
  cairo_set_source_rgb (cairo, fill_color.r, fill_color.g, fill_color.b);
  cairo_mask (cairo, pattern);

  cairo_pattern_destroy (pattern);
  cairo_surface_destroy (image);
  free (buffer);
  delete imgStr;
#endif
}

//TODO: lots more work here.
void ArthurOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
				int width, int height,
				GfxImageColorMap *colorMap,
				int *maskColors, GBool inlineImg)
{
  unsigned char *buffer;
  unsigned int *dest;
  int x, y;
  ImageStream *imgStr;
  Guchar *pix;
  GfxRGB rgb;
  int alpha, i;
  double *ctm;
  QMatrix matrix;
  int is_identity_transform;
  
  buffer = (unsigned char *)gmalloc (width * height * 4);

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width,
			   colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
  
  /* ICCBased color space doesn't do any color correction
   * so check its underlying color space as well */
  is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
		  colorMap->getColorSpace()->getMode() == csICCBased && 
		  ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB;

  if (maskColors) {
    for (y = 0; y < height; y++) {
      dest = (unsigned int *) (buffer + y * 4 * width);
      pix = imgStr->getLine();
      colorMap->getRGBLine (pix, dest, width);

      for (x = 0; x < width; x++) {
	for (i = 0; i < colorMap->getNumPixelComps(); ++i) {
	  
	  if (pix[i] < maskColors[2*i] * 255||
	      pix[i] > maskColors[2*i+1] * 255) {
	    *dest = *dest | 0xff000000;
	    break;
	  }
	}
	pix += colorMap->getNumPixelComps();
	dest++;
      }
    }

    m_image = new QImage(buffer, width, height, QImage::Format_ARGB32);
  }
  else {
    for (y = 0; y < height; y++) {
      dest = (unsigned int *) (buffer + y * 4 * width);
      pix = imgStr->getLine();
      colorMap->getRGBLine (pix, dest, width);
    }

    m_image = new QImage(buffer, width, height, QImage::Format_RGB32);
  }

  if (m_image == NULL || m_image->isNull()) {
    qDebug() << "Null image";
    return;
  }
  ctm = state->getCTM();
  matrix.setMatrix(ctm[0] / width, ctm[1] / width, -ctm[2] / height, -ctm[3] / height, ctm[2] + ctm[4], ctm[3] + ctm[5]);

  m_painter->setMatrix(matrix, true);
  m_painter->drawImage( QPoint(0,0), *m_image );
  free (buffer);
  delete imgStr;

}
