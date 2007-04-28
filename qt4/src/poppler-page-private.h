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

#ifndef _POPPLER_PAGE_PRIVATE_H_
#define _POPPLER_PAGE_PRIVATE_H_

#include "poppler-private.h"

namespace Poppler
{

class PageData {
public:
  Link* convertLinkActionToLink(::LinkAction * a, const QRectF &linkArea, DocumentData * doc);

  const Document *parentDoc;
  int index;
  PageTransition *transition;
};

}

#endif
