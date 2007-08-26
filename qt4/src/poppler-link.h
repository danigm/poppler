/* poppler-link.h: qt interface to poppler
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

#ifndef _POPPLER_LINK_H_
#define _POPPLER_LINK_H_

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QSharedDataPointer>

namespace Poppler {

class LinkPrivate;
class LinkGotoPrivate;
class LinkExecutePrivate;
class LinkBrowsePrivate;
class LinkActionPrivate;
class LinkSoundPrivate;
class LinkMoviePrivate;
class LinkDestinationData;
class LinkDestinationPrivate;
class SoundObject;

/**
 * \short Encapsulates data that describes a link destination.
 *
 * Coordinates are in 0..1 range
 */
class LinkDestination
{
	public:
		enum Kind
		{
			destXYZ = 1,
			destFit = 2,
			destFitH = 3,
			destFitV = 4,
			destFitR = 5,
			destFitB = 6,
			destFitBH = 7,
			destFitBV = 8
		};

		LinkDestination(const LinkDestinationData &data);
		LinkDestination(const QString &description);
		LinkDestination(const LinkDestination &other);
		~LinkDestination();

		// Accessors.
		Kind kind() const;
		int pageNumber() const;
		double left() const;
		double bottom() const;
		double right() const;
		double top() const;
		double zoom() const;
		bool isChangeLeft() const;
		bool isChangeTop() const;
		bool isChangeZoom() const;

		QString toString() const;

		LinkDestination& operator=(const LinkDestination &other);

	private:
		QSharedDataPointer< LinkDestinationPrivate > d;
};

/**
 * \short Encapsulates data that describes a link.
 *
 * This is the base class for links. It makes mandatory for inherited
 * kind of links to reimplement the linkType() method and return the type of
 * the link described by the reimplemented class.
 */
class Link
{
	public:
		Link( const QRectF &linkArea );
		
		/**
		 * The possible kinds of link.
		 *
		 * \internal
		 * Inherited classes must return an unique identifier
		 */
		enum LinkType
		{
		    None,     ///< Unknown link
		    Goto,     ///< A "Go To" link
		    Execute,
		    Browse,
		    Action,
		    Sound,    ///< A link representing a sound to be played
		    Movie
		};

		/**
		 * The type of this link.
		 */
		virtual LinkType linkType() const;

		// virtual destructor
		virtual ~Link();
		
		/**
		 * The area of a Page where the link should be active.
		 *
		 * \note this can be a null rect, in this case the link represents
		 * a general action. The area is given in 0..1 range
		 */
		QRectF linkArea() const;
		
	protected:
		Link( LinkPrivate &dd );
		Q_DECLARE_PRIVATE( Link )
		LinkPrivate *d_ptr;
		
	private:
		Q_DISABLE_COPY( Link )
};


/** Goto: a viewport and maybe a reference to an external filename **/
class LinkGoto : public Link
{
	public:
		LinkGoto( const QRectF &linkArea, QString extFileName, const LinkDestination & destination );
		~LinkGoto();

		/**
		 * Whether the destination is in an external document
		 * (i.e. not the current document)
		 */
		bool isExternal() const;
		// query for goto parameters
		QString fileName() const;
		LinkDestination destination() const;
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkGoto )
		Q_DISABLE_COPY( LinkGoto )
};

/** Execute: filename and parameters to execute **/
class LinkExecute : public Link
{
	public:
		/**
		 * The file name to be executed
		 */
		QString fileName() const;
		QString parameters() const;

		// create a Link_Execute
		LinkExecute( const QRectF &linkArea, const QString & file, const QString & params );
		~LinkExecute();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkExecute )
		Q_DISABLE_COPY( LinkExecute )
};

/** Browse: an URL to open, ranging from 'http://' to 'mailto:', etc. **/
class LinkBrowse : public Link
{
	public:
		/**
		 * The URL to open
		 */
		QString url() const;

		// create a Link_Browse
		LinkBrowse( const QRectF &linkArea, const QString &url );
		~LinkBrowse();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkBrowse )
		Q_DISABLE_COPY( LinkBrowse )
};	

/** Action: contains an action to perform on document / viewer **/
class LinkAction : public Link
{
	public:
		/**
		 * The possible types of actions
		 */
		enum ActionType { PageFirst = 1,
		                  PagePrev = 2,
		                  PageNext = 3,
		                  PageLast = 4,
		                  HistoryBack = 5,
		                  HistoryForward = 6,
		                  Quit = 7,
		                  Presentation = 8,
		                  EndPresentation = 9,
		                  Find = 10,
		                  GoToPage = 11,
		                  Close = 12 };

		/**
		 * The action of the current LinkAction
		 */
		ActionType actionType() const;

		// create a Link_Action
		LinkAction( const QRectF &linkArea, ActionType actionType );
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkAction )
		Q_DISABLE_COPY( LinkAction )
};

/** Sound: a sound to be played **/
class LinkSound : public Link
{
	public:
		// create a Link_Sound
		LinkSound( const QRectF &linkArea, double volume, bool sync, bool repeat, bool mix, SoundObject *sound );
		virtual ~LinkSound();

		LinkType linkType() const;

		/**
		 * The volume to be used when playing the sound.
		 *
		 * The volume is in the range [ -1, 1 ], where:
		 * - a negative number: no volume (mute)
		 * - 1: full volume
		 */
		double volume() const;
		bool synchronous() const;
		/**
		 * Whether the sound should be played continuously (that is,
		 * started again when it ends)
		 */
		bool repeat() const;
		bool mix() const;
		/**
		 * The sound object to be played
		 */
		SoundObject *sound() const;

	private:
		Q_DECLARE_PRIVATE( LinkSound )
		Q_DISABLE_COPY( LinkSound )
};

/** Movie: Not yet defined -> think renaming to 'Media' link **/
class LinkMovie : public Link
// TODO this (Movie link)
{
	public:
		LinkMovie( const QRectF &linkArea );
		~LinkMovie();
		LinkType linkType() const;

	private:
		Q_DECLARE_PRIVATE( LinkMovie )
		Q_DISABLE_COPY( LinkMovie )
};

}

#endif
