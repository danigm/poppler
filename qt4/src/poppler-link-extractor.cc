/* poppler-link-extractor_p.h: qt interface to poppler
 * Copyright (C) 2007, Pino Toscano <pino@kde.org>
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

#include <Object.h>

#include "poppler-qt4.h"
#include "poppler-link-extractor-private.h"
#include "poppler-page-private.h"
#include "poppler-private.h"

namespace Poppler
{

LinkExtractorOutputDev::LinkExtractorOutputDev(PageData *data, DocumentData *doc)
  : m_data(data), m_doc(doc)
{
  m_popplerPage = m_doc->doc->getCatalog()->getPage(m_data->index + 1);
  GfxState gfxState(72.0, 72.0, m_popplerPage->getCropBox(), m_popplerPage->getRotate(), gTrue);
  setDefaultCTM(gfxState.getCTM());
}

LinkExtractorOutputDev::~LinkExtractorOutputDev()
{
  qDeleteAll(m_links);
}

void LinkExtractorOutputDev::processLink(::Link *link, Catalog *catalog)
{
  if (!link->isOk() || !m_data)
    return;

  double left, top, right, bottom;
  int leftAux, topAux, rightAux, bottomAux;
  link->getRect(&left, &top, &right, &bottom);
  QRectF linkArea;

  cvtUserToDev(left, top, &leftAux, &topAux);
  cvtUserToDev(right, bottom, &rightAux, &bottomAux);
  linkArea.setLeft((double)leftAux / (double)m_popplerPage->getCropWidth());
  linkArea.setTop((double)topAux / (double)m_popplerPage->getCropHeight());
  linkArea.setRight((double)rightAux / (double)m_popplerPage->getCropWidth());
  linkArea.setBottom((double)bottomAux / (double)m_popplerPage->getCropHeight());

  Link *popplerLink = m_data->convertLinkActionToLink(link->getAction(), linkArea, m_doc);
  if (popplerLink)
  {
    m_links.append(popplerLink);
  }
  OutputDev::processLink(link, catalog);
}

QList< Link* > LinkExtractorOutputDev::links()
{
  QList< Link* > ret = m_links;
  m_links.clear();
  return ret;
}

}
