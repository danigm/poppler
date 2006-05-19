/* poppler-link.cc: qt interface to poppler
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

#define UNSTABLE_POPPLER_QT4

#include <poppler-qt4.h>
#include <poppler-private.h>

#include <QtCore/QStringList>

#include "Link.h"

namespace Poppler {

	LinkDestination::LinkDestination(const LinkDestinationData &data)
	{
		LinkDest *ld = data.ld;
		
		if ( data.namedDest && !ld )
			ld = data.doc->doc.findDest( data.namedDest );
		
		if (!ld) return;
		
		if (ld->getKind() == ::destXYZ) m_kind = destXYZ;
		else if (ld->getKind() == ::destFit) m_kind = destFit;
		else if (ld->getKind() == ::destFitH) m_kind = destFitH;
		else if (ld->getKind() == ::destFitV) m_kind = destFitV;
		else if (ld->getKind() == ::destFitR) m_kind = destFitR;
		else if (ld->getKind() == ::destFitB) m_kind = destFitB;
		else if (ld->getKind() == ::destFitBH) m_kind = destFitBH;
		else if (ld->getKind() == ::destFitBV) m_kind = destFitBV;

		if ( !ld->isPageRef() ) m_pageNum = ld->getPageNum();
		else
		{
			Ref ref = ld->getPageRef();
			m_pageNum = data.doc->doc.findPage( ref.num, ref.gen );
		}
		double left = ld->getLeft();
		double bottom = ld->getBottom();
		double right = ld->getRight();
		double top = ld->getTop();
		m_zoom = ld->getZoom();
		m_changeLeft = ld->getChangeLeft();
		m_changeTop = ld->getChangeTop();
		m_changeZoom = ld->getChangeZoom();
		
		int leftAux, topAux, rightAux, bottomAux;
		
		SplashOutputDev *sod = data.doc->getSplashOutputDev();
		sod->cvtUserToDev( left, top, &leftAux, &topAux );
		sod->cvtUserToDev( right, bottom, &rightAux, &bottomAux );
		
		m_left = leftAux;
		m_top = topAux;
		m_right = rightAux;
		m_bottom = bottomAux;
	}
	
	LinkDestination::LinkDestination(const QString &description)
	{
		QStringList tokens = description.split( ';' );
		m_kind = static_cast<Kind>(tokens.at(0).toInt());
		m_pageNum = tokens.at(1).toInt();
		m_left = tokens.at(2).toDouble();
		m_bottom = tokens.at(3).toDouble();
		m_top = tokens.at(4).toDouble();
		m_zoom = tokens.at(5).toDouble();
		m_changeLeft = static_cast<bool>(tokens.at(6).toInt());
		m_changeTop = static_cast<bool>(tokens.at(7).toInt());
		m_changeZoom = static_cast<bool>(tokens.at(8).toInt());
	}
	
	LinkDestination::Kind LinkDestination::kind() const
	{
		return m_kind;
	}
	
	int LinkDestination::pageNumber() const
	{
		return m_pageNum;
	}
	
	double LinkDestination::left() const
	{
		return m_left;
	}
	
	double LinkDestination::bottom() const
	{
		return m_bottom;
	}
	
	double LinkDestination::right() const
	{
		return m_right;
	}
	
	double LinkDestination::top() const
	{
		return m_top;
	}
	
	double LinkDestination::zoom() const
	{
		return m_zoom;
	}
	
	bool LinkDestination::isChangeLeft() const
	{
		return m_changeLeft;
	}
	
	bool LinkDestination::isChangeTop() const
	{
		return m_changeTop;
	}
	
	bool LinkDestination::isChangeZoom() const
	{
		return m_changeZoom;
	}
	
	QString LinkDestination::toString() const
	{
		QString s = QString::number( (qint8)m_kind );
		s += ";" + QString::number( m_pageNum );
		s += ";" + QString::number( m_left );
		s += ";" + QString::number( m_bottom );
		s += ";" + QString::number( m_right );
		s += ";" + QString::number( m_top );
		s += ";" + QString::number( m_zoom );
		s += ";" + QString::number( (qint8)m_changeLeft );
		s += ";" + QString::number( (qint8)m_changeTop );
		s += ";" + QString::number( (qint8)m_changeZoom );
		return s;
	}
	
	
	// Link
	Link::~Link()
	{
	}
	
	Link::Link(const QRectF &linkArea) : m_linkArea(linkArea)
	{
	}
	
	Link::LinkType Link::linkType() const
	{
		return None;
	}
	
	QRectF Link::linkArea() const
	{
		return m_linkArea;
	}
	
	// LinkGoto
	LinkGoto::LinkGoto( const QRectF &linkArea, QString extFileName, const LinkDestination & destination ) : Link(linkArea), m_extFileName(extFileName), m_destination(destination)
	{
	}
	
	bool LinkGoto::isExternal() const
	{
		return !m_extFileName.isEmpty();
	}
	
	const QString &LinkGoto::fileName() const
	{
		return m_extFileName;
	}
	
	const LinkDestination &LinkGoto::destination() const
	{
		return m_destination;
	}
	
	Link::LinkType LinkGoto::linkType() const
	{
		return Goto;
	}
	
	// LinkExecute
	LinkExecute::LinkExecute( const QRectF &linkArea, const QString & file, const QString & params ) : Link(linkArea), m_fileName(file), m_parameters(params)
	{
	}
	
	const QString & LinkExecute::fileName() const
	{
		return m_fileName;
	}
	const QString & LinkExecute::parameters() const
	{
		return m_parameters;
	}

	Link::LinkType LinkExecute::linkType() const
	{
		return Execute;
	}

	// LinkBrowse
	LinkBrowse::LinkBrowse( const QRectF &linkArea, const QString &url ) : Link(linkArea), m_url(url)
	{
	}
	
	const QString & LinkBrowse::url() const
	{
		return m_url;
	}
	
	Link::LinkType LinkBrowse::linkType() const
	{
		return Browse;
	}

	// LinkAction
	LinkAction::LinkAction( const QRectF &linkArea, ActionType actionType ) : Link(linkArea), m_type(actionType)
	{
	}
		
	LinkAction::ActionType LinkAction::actionType() const
	{
		return m_type;
	}

	Link::LinkType LinkAction::linkType() const
	{
		return Action;
	}

	// LinkMovie
	LinkMovie::LinkMovie( const QRectF &linkArea ) : Link(linkArea)
	{
	}
	
	Link::LinkType LinkMovie::linkType() const
	{
		return Movie;
	}

}
