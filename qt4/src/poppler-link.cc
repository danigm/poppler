/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2006, Albert Astals Cid
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
			m_pageNum = data.doc->findPage( ref.num, ref.gen );
		}
		m_left = ld->getLeft();
		m_bottom = ld->getBottom();
		m_right = ld->getRight();
		m_top = ld->getTop();
		m_zoom = ld->getZoom();
		m_changeLeft = ld->getChangeLeft();
		m_changeTop = ld->getChangeTop();
		m_changeZoom = ld->getChangeZoom();
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

}
