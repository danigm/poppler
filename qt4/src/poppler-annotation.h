/* poppler-annotation.h: qt interface to poppler
 * Copyright (C) 2006, Albert Astals Cid
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

#ifndef _POPPLER_ANNOTATION_H_
#define _POPPLER_ANNOTATION_H_

#include <QtCore/QDateTime>
#include <QtCore/QLinkedList>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QFont>

namespace Poppler {

class Annotation;
class Link;

/**
 * @short Helper class for (recoursive) annotation retrieval/storage.
 *
 */
class AnnotationUtils
{
    public:
        // restore an annotation (with revisions if needed) from a dom
        // element. returns a pointer to the complete annotation or 0 if
        // element is invalid.
        static Annotation * createAnnotation( const QDomElement & annElement );

        // save the 'ann' annotations as a child of parentElement taking
        // care of saving all revisions if 'ann' has any.
        static void storeAnnotation( const Annotation * ann,
            QDomElement & annElement, QDomDocument & document );

        // return an element called 'name' from the direct children of
        // parentNode or a null element if not found
        static QDomElement findChildElement( const QDomNode & parentNode,
            const QString & name );

        //static inline QRect annotationGeometry( const Annotation * ann,
        //    int pageWidth, int pageHeight, int scaledWidth, int scaledHeight ) const;
};


/**
 * @short Annotation struct holds properties shared by all annotations.
 *
 * An Annotation is an object (text note, highlight, sound, popup window, ..)
 * contained by a Page in the document.
 *
 */
struct Annotation
{
    // enum definitions
    // WARNING!!! oKular uses that very same values so if you change them notify the author!
    enum SubType { AText = 1, ALine = 2, AGeom = 3, AHighlight = 4, AStamp = 5,
                   AInk = 6, ALink = 7, A_BASE = 0 };
    enum Flag { Hidden = 1, FixedSize = 2, FixedRotation = 4, DenyPrint = 8,
                DenyWrite = 16, DenyDelete = 32, ToggleHidingOnMouse = 64, External = 128 };
    enum LineStyle { Solid = 1, Dashed = 2, Beveled = 4, Inset = 8, Underline = 16 };
    enum LineEffect { NoEffect = 1, Cloudy = 2};
    enum RevScope { Reply = 1, Group = 2, Delete = 4 };
    enum RevType { None = 1,  Marked = 2, Unmarked = 4,  Accepted = 8, Rejected = 16, Cancelled = 32, Completed = 64 };


    /** properties: contents related */
    QString         author;                 // ''
    QString         contents;               // ''
    QString         uniqueName;             // '#NUMBER#'
    QDateTime       modifyDate;             // before or equal to currentDateTime()
    QDateTime       creationDate;           // before or equal to modifyDate

    /** properties: look/interaction related */
    int             flags;                  // 0
    QRectF          boundary;               // valid or isNull()
    struct Style
    {
        // appearance properties
        QColor          color;              // black
        double          opacity;            // 1.0
        // pen properties
        double          width;              // 1.0
        LineStyle       style;              // LineStyle::Solid
        double          xCorners;           // 0.0
        double          yCorners;           // 0.0
        int             marks;              // 3
        int             spaces;             // 0
        // pen effects
        LineEffect      effect;             // LineEffect::NoEffect
        double          effectIntensity;    // 1.0
        // default initializer
        Style();
    }               style;

    /** properties: popup window */
    struct Window
    {
        // window state (Hidden, FixedRotation, Deny* flags allowed)
        int             flags;              // -1 (never initialized) -> 0 (if inited and shown)
        // geometric properties
        QPointF         topLeft;            // no default, inited to boundary.topLeft
        int             width;              // no default
        int             height;             // no default
        // window contens/override properties
        QString         title;              // '' text in the titlebar (overrides author)
        QString         summary;            // '' short description (displayed if not empty)
        QString         text;               // '' text for the window (overrides annot->contents)
        // default initializer
        Window();
    }               window;

    /** properties: versioning */
    struct Revision
    {
        // child revision
        Annotation *    annotation;         // not null
        // scope and type of revision
        RevScope        scope;              // Reply
        RevType         type;               // None
        // default initializer
        Revision();
    };
    QLinkedList< Revision > revisions;       // empty by default

    // methods: query annotation's type for runtime type identification
    virtual SubType subType() const { return A_BASE; }
    //QRect geometry( int scaledWidth, int scaledHeight, KPDFPage * page );

    // methods: storage/retrieval from xml nodes
    Annotation( const QDomNode & node );
    virtual void store( QDomNode & parentNode, QDomDocument & document ) const;

    // methods: default constructor / virtual destructor
    Annotation();
    virtual ~Annotation();
};


// a helper used to shorten the code presented below
#define AN_COMMONDECL( className, rttiType )\
    className();\
    className( const class QDomNode & node );\
    void store( QDomNode & parentNode, QDomDocument & document ) const;\
    SubType subType() const { return rttiType; }

struct TextAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( TextAnnotation, AText );

    // local enums
    enum TextType { Linked, InPlace };
    enum InplaceIntent { Unknown, Callout, TypeWriter };

    // data fields
    TextType        textType;               // Linked
    QString         textIcon;               // 'Note'
    QFont           textFont;               // app def font
    int             inplaceAlign;           // 0:left, 1:center, 2:right
    QString         inplaceText;            // '' overrides contents
    QPointF         inplaceCallout[3];      //
    InplaceIntent   inplaceIntent;          // Unknown
};

struct LineAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( LineAnnotation, ALine )

    // local enums
    enum TermStyle { Square, Circle, Diamond, OpenArrow, ClosedArrow, None,
                     Butt, ROpenArrow, RClosedArrow, Slash };
    enum LineIntent { Unknown, Arrow, Dimension, PolygonCloud };

    // data fields (note uses border for rendering style)
    QLinkedList<QPointF> linePoints;
    TermStyle       lineStartStyle;         // None
    TermStyle       lineEndStyle;           // None
    bool            lineClosed;             // false (if true draw close shape)
    QColor          lineInnerColor;         //
    double          lineLeadingFwdPt;       // 0.0
    double          lineLeadingBackPt;      // 0.0
    bool            lineShowCaption;        // false
    LineIntent      lineIntent;             // Unknown
};

struct GeomAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( GeomAnnotation, AGeom )

    // common enums
    enum GeomType { InscribedSquare, InscribedCircle };

    // data fields (note uses border for rendering style)
    GeomType        geomType;               // InscribedSquare
    QColor          geomInnerColor;         //
    int             geomWidthPt;            // 18
};

struct HighlightAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( HighlightAnnotation, AHighlight )

    // local enums
    enum HighlightType { Highlight, Squiggly, Underline, StrikeOut };

    // data fields
    HighlightType   highlightType;          // Highlight
    struct Quad
    {
        QPointF         points[4];          // 8 valid coords
        bool            capStart;           // false (vtx 1-4) [K]
        bool            capEnd;             // false (vtx 2-3) [K]
        double          feather;            // 0.1 (in range 0..1) [K]
    };
    QList< Quad >  highlightQuads;     // not empty
};

struct StampAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( StampAnnotation, AStamp )

    // data fields
    QString         stampIconName;          // 'Draft'
};

struct InkAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( InkAnnotation, AInk )

    // data fields
    QList< QLinkedList<QPointF> > inkPaths;
};

struct LinkAnnotation : public Annotation
{
    // common stuff for Annotation derived classes
    AN_COMMONDECL( LinkAnnotation, ALink );
    virtual ~LinkAnnotation();

    // local enums
    enum HighlightMode { None, Invert, Outline, Push };

    // data fields
    Link *          linkDestination;        //
    HighlightMode   linkHLMode;             // Invert
    QPointF         linkRegion[4];          //
};

}

#endif
