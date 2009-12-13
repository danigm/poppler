/*
 * Copyright (C) 2009, Pino Toscano <pino@kde.org>
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
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "poppler-page.h"
#include "poppler-page-transition.h"

#include "poppler-document-private.h"
#include "poppler-page-private.h"
#include "poppler-private.h"

using namespace poppler;

page_private::page_private(document_private *_doc, int _index)
    : doc(_doc)
    , page(doc->doc->getCatalog()->getPage(index + 1))
    , index(_index)
    , transition(0)
{
}

page_private::~page_private()
{
    delete transition;
}


page::page(document_private *doc, int index)
    : d(new page_private(doc, index))
{
}

page::~page()
{
    delete d;
}

page::orientation_enum page::orientation() const
{
    const int rotation = d->page->getRotate();
    switch (rotation) {
    case 90:
        return landscape;
        break;
    case 180:
        return upside_down;
        break;
    case 270:
        return seascape;
        break;
    default:
        return portrait;
    }
}

double page::duration() const
{
    return d->page->getDuration();
}

rectf page::page_rect(page_box_enum box) const
{
    PDFRectangle *r = 0;
    switch (box) {
    case media_box:
        r = d->page->getMediaBox();
        break;
    case crop_box:
        r = d->page->getCropBox();
        break;
    case bleed_box:
        r = d->page->getBleedBox();
        break;
    case trim_box:
        r = d->page->getTrimBox();
        break;
    case art_box:
        r = d->page->getArtBox();
        break;
    }
    if (r) {
        return detail::pdfrectangle_to_rectf(*r);
    }
    return rectf();
}

ustring page::label() const
{
    GooString goo;
    if (!d->doc->doc->getCatalog()->indexToLabel(d->index, &goo)) {
        return ustring();
    }

    return detail::unicode_GooString_to_ustring(&goo);
}

page_transition* page::transition() const
{
    if (!d->transition) {
        Object o;
        if (d->page->getTrans(&o)->isDict()) {
            d->transition = new page_transition(&o);
        }
        o.free();
    }
    return d->transition;
}
