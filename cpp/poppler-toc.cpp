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

#include "poppler-toc.h"

#include "poppler-toc-private.h"
#include "poppler-private.h"

#include "GooList.h"
#include "Outline.h"

using namespace poppler;

toc_private::toc_private()
{
}

toc_private::~toc_private()
{
}

toc* toc_private::load_from_outline(Outline *outline)
{
    if (!outline) {
        return 0;
    }

    GooList *items = outline->getItems();
    if (!items || items->getLength() < 1) {
        return 0;
    }

    toc *newtoc = new toc();
    newtoc->d->root.d->is_open = true;
    newtoc->d->root.d->load_children(items);

    return newtoc;
}

toc_item_private::toc_item_private()
    : is_open(false)
{
}

toc_item_private::~toc_item_private()
{
    delete_all(children);
}

void toc_item_private::load(OutlineItem *item)
{
    const Unicode *title_unicode = item->getTitle();
    const int title_length = item->getTitleLength();
    title = detail::unicode_to_ustring(title_unicode, title_length);
    is_open = item->isOpen();
}

void toc_item_private::load_children(GooList *items)
{
    const int num_items = items->getLength();
    children.resize(num_items);
    for (int i = 0; i < num_items; ++i) {
        OutlineItem *item = (OutlineItem *)items->get(i);

        toc_item *new_item = new toc_item();
        new_item->d->load(item);
        children[i] = new_item;

        item->open();
        GooList *item_children = item->getKids();
        if (item_children) {
            new_item->d->load_children(item_children);
        }
    }
}


toc::toc()
    : d(new toc_private())
{
}

toc::~toc()
{
    delete d;
}

toc_item* toc::root() const
{
    return &d->root;
}


toc_item::toc_item()
    : d(new toc_item_private())
{
}

toc_item::~toc_item()
{
    delete d;
}

ustring toc_item::title() const
{
    return d->title;
}

bool toc_item::is_open() const
{
    return d->is_open;
}

std::vector<toc_item *> toc_item::children() const
{
    return d->children;
}

toc_item::iterator toc_item::children_begin() const
{
    return d->children.begin();
}

toc_item::iterator toc_item::children_end() const
{
    return d->children.end();
}
