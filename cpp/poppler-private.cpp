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

#include "poppler-private.h"

#include "GooString.h"
#include "Page.h"

#include <iostream>
#include <sstream>

using namespace poppler;

void detail::error_function(int pos, char *msg, va_list args)
{
    std::ostringstream oss;
    if (pos >= 0) {
        oss << "poppler/error (" << pos << "): ";
    } else {
        oss << "poppler/error: ";
    }
    char buffer[4096]; // should be big enough
    vsnprintf(buffer, sizeof(buffer) - 1, msg, args);
    oss << buffer;
    std::cerr << oss.str();
}

rectf detail::pdfrectangle_to_rectf(const PDFRectangle &pdfrect)
{
    return rectf(pdfrect.x1, pdfrect.y1, pdfrect.x2 - pdfrect.x1, pdfrect.y2 - pdfrect.y1);
}

ustring detail::unicode_GooString_to_ustring(GooString *str)
{
    return ustring::from_utf_8(str->getCString(), str->getLength());
}

ustring detail::unicode_to_ustring(const Unicode *u, int length)
{
    ustring str(length, 0);
    ustring::iterator it = str.begin(), it_end = str.end();
    const Unicode *uu = u;
    for (; it != it_end; ++it) {
        *it = ustring::value_type(*uu++);
    }
    return str;
}

GooString* detail::ustring_to_unicode_GooString(const ustring &str)
{
    const byte_array utf8_data = str.to_utf_8();
    GooString *goo = new GooString(utf8_data.data());
    return goo;
}
