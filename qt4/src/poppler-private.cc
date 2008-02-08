/* poppler-private.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2006 by Albert Astals Cid <aacid@kde.org>
 * Inspired on code by
 * Copyright (C) 2004 by Albert Astals Cid <tsdgeos@terra.es>
 * Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

namespace Poppler {

    QString unicodeToQString(Unicode* u, int len) {
        QString ret;
        ret.resize(len);
        QChar* qch = (QChar*) ret.unicode();
        for (;len;--len)
          *qch++ = (QChar) *u++;
        return ret;
    }

    QString UnicodeParsedString(GooString *s1) {
        if ( !s1 || s1->getLength() == 0 )
            return QString();

        GBool isUnicode;
        int i;
        Unicode u;
        QString result;
        if ( ( s1->getChar(0) & 0xff ) == 0xfe && ( s1->getLength() > 1 && ( s1->getChar(1) & 0xff ) == 0xff ) )
        {
            isUnicode = gTrue;
            i = 2;
        }
        else
        {
            isUnicode = gFalse;
            i = 0;
        }
        while ( i < s1->getLength() )
        {
            if ( isUnicode )
            {
                u = ( ( s1->getChar(i) & 0xff ) << 8 ) | ( s1->getChar(i+1) & 0xff );
                i += 2;
            }
            else
            {
                u = s1->getChar(i) & 0xff;
                ++i;
            }
            result += unicodeToQString( &u, 1 );
        }
        return result;
    }
}
