/* poppler-annotation-helper.h: qt interface to poppler
 * Copyright (C) 2006, Albert Astals Cid <aacid@kde.org>
 * Adapting code from
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#include <QtCore/QDebug>

namespace Poppler {

class XPDFReader
{
    public:
        // find named symbol and parse it
        static void lookupName( Dict *, const char *, QString & dest );
        static void lookupString( Dict *, const char *, QString & dest );
        static void lookupBool( Dict *, const char *, bool & dest );
        static void lookupInt( Dict *, const char *, int & dest );
        static void lookupNum( Dict *, const char *, double & dest );
        static int lookupNumArray( Dict *, const char *, double * dest, int len );
        static void lookupColor( Dict *, const char *, QColor & color );
        static void lookupIntRef( Dict *, const char *, int & dest );
        static void lookupDate( Dict *, const char *, QDateTime & dest );
        // transform from user coords to normalized ones using the matrix M
        static inline void transform( double * M, double x, double y, QPointF &res );
};

void XPDFReader::lookupName( Dict * dict, const char * type, QString & dest )
{
    Object nameObj;
    dict->lookup( type, &nameObj );
    if ( nameObj.isNull() )
        return;
    if ( nameObj.isName() )
        dest = nameObj.getName();
    else
        qDebug() << type << " is not Name." << endl;
    nameObj.free();
}

void XPDFReader::lookupString( Dict * dict, const char * type, QString & dest )
{
    Object stringObj;
    dict->lookup( type, &stringObj );
    if ( stringObj.isNull() )
        return;
    if ( stringObj.isString() )
        dest = stringObj.getString()->getCString();
    else
        qDebug() << type << " is not String." << endl;
    stringObj.free();
}

void XPDFReader::lookupBool( Dict * dict, const char * type, bool & dest )
{
    Object boolObj;
    dict->lookup( type, &boolObj );
    if ( boolObj.isNull() )
        return;
    if ( boolObj.isBool() )
        dest = boolObj.getBool() == gTrue;
    else
        qDebug() << type << " is not Bool." << endl;
    boolObj.free();
}

void XPDFReader::lookupInt( Dict * dict, const char * type, int & dest )
{
    Object intObj;
    dict->lookup( type, &intObj );
    if ( intObj.isNull() )
        return;
    if ( intObj.isInt() )
        dest = intObj.getInt();
    else
        qDebug() << type << " is not Int." << endl;
    intObj.free();
}

void XPDFReader::lookupNum( Dict * dict, const char * type, double & dest )
{
    Object numObj;
    dict->lookup( type, &numObj );
    if ( numObj.isNull() )
        return;
    if ( numObj.isNum() )
        dest = numObj.getNum();
    else
        qDebug() << type << " is not Num." << endl;
    numObj.free();
}

int XPDFReader::lookupNumArray( Dict * dict, const char * type, double * dest, int len )
{
    Object arrObj;
    dict->lookup( type, &arrObj );
    if ( arrObj.isNull() )
        return 0;
    Object numObj;
    if ( arrObj.isArray() )
    {
        len = qMin( len, arrObj.arrayGetLength() );
        for ( int i = 0; i < len; i++ )
        {
            dest[i] = arrObj.arrayGet( i, &numObj )->getNum();
            numObj.free();
        }
    }
    else
    {
        len = 0;
        qDebug() << type << "is not Array." << endl;
    }
    arrObj.free();
    return len;
}

void XPDFReader::lookupColor( Dict * dict, const char * type, QColor & dest )
{
    double c[3];
    if ( XPDFReader::lookupNumArray( dict, type, c, 3 ) == 3 )
        dest = QColor( (int)(c[0]*255.0), (int)(c[1]*255.0), (int)(c[2]*255.0));
}

void XPDFReader::lookupIntRef( Dict * dict, const char * type, int & dest )
{
    Object refObj;
    dict->lookupNF( type, &refObj );
    if ( refObj.isNull() )
        return;
    if ( refObj.isRef() )
        dest = refObj.getRefNum();
    else
        qDebug() << type << " is not Ref." << endl;
    refObj.free();
}

void XPDFReader::lookupDate( Dict * dict, const char * type, QDateTime & dest )
{
    Object dateObj;
    dict->lookup( type, &dateObj );
    if ( dateObj.isNull() )
        return;
    if ( dateObj.isString() )
    {
        const char * s = dateObj.getString()->getCString();
        if ( s[0] == 'D' && s[1] == ':' )
            s += 2;
        int year, mon, day, hour, min, sec;
        if ( sscanf( s, "%4d%2d%2d%2d%2d%2d", &year, &mon, &day, &hour, &min, &sec ) == 6 )
        {
            QDate d( year, mon, day );
            QTime t( hour, min, sec );
            if ( d.isValid() && t.isValid() )
                dest = QDateTime(d, t);
        }
        else
            qDebug() << "Wrong Date format '" << s << "' for '" << type << "'." << endl;
    }
    else
        qDebug() << type << " is not Date" << endl;
    dateObj.free();
}

void XPDFReader::transform( double * M, double x, double y, QPointF &res )
{
    res.setX( M[0] * x + M[2] * y + M[4] );
    res.setY( M[1] * x + M[3] * y + M[5] );
}

/** @short Helper classes for CROSSDEPS resolving and DS conversion. */
struct ResolveRevision
{
    int           prevAnnotationID; // ID of the annotation to be reparended
    int           nextAnnotationID; // (only needed for speeding up resolving)
    Annotation *  nextAnnotation;   // annotation that will act as parent
    Annotation::RevScope nextScope; // scope of revision (Reply)
    Annotation::RevType  nextType;  // type of revision (None)
};

struct ResolveWindow
{
    int           popupWindowID;    // ID of the (maybe shared) window
    Annotation *  annotation;       // annotation having the popup window
};

struct PostProcessText              // this handles a special pdf case conversion
{
    Annotation *  textAnnotation;   // a popup text annotation (not FreeText)
    bool          opened;           // pdf property to convert to window flags
};

struct PopupWindow
{
    Annotation *  dummyAnnotation;  // window properties (in pdf as Annotation)
    bool          shown;            // converted to Annotation::Hidden flag
};

}
