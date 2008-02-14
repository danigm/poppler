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

#include <QtCore/QByteArray>
#include <QtCore/QDebug>

namespace Poppler {

    void qt4ErrorFunction(int pos, char *msg, va_list args)
    {
        QString emsg;
        char buffer[1024]; // should be big enough

        if (pos >= 0)
        {
            emsg = QString::fromLatin1("Error (%1): ").arg(pos);
        }
        else
        {
            emsg = QString::fromLatin1("Error: ");
        }
        qvsnprintf(buffer, sizeof(buffer) - 1, msg, args);
        emsg += QString::fromAscii(buffer);
        qDebug() << qPrintable(emsg);
    }

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

    GooString *QStringToUnicodeGooString(const QString &s) {
        int len = s.length() * 2 + 2;
        char *cstring = (char *)gmallocn(len, sizeof(char));
        cstring[0] = 0xfe;
        cstring[1] = 0xff;
        for (int i = 0; i < s.length(); ++i)
        {
            cstring[2+i*2] = s.at(i).row();
            cstring[3+i*2] = s.at(i).cell();
        }
        GooString *ret = new GooString(cstring, len);
        gfree(cstring);
        return ret;
    }

    GooString *QStringToGooString(const QString &s) {
        int len = s.length();
        char *cstring = (char *)gmallocn(s.length(), sizeof(char));
        for (int i = 0; i < len; ++i)
            cstring[i] = s.at(i).unicode();
        GooString *ret = new GooString(cstring, len);
        gfree(cstring);
        return ret;
    }
}
