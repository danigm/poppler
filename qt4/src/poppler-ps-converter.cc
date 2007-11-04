/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2007, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-qt4.h"
#include "poppler-private.h"

#include "PSOutputDev.h"

#include <QtCore/QFile>

static void outputToQIODevice(void *stream, char *data, int len)
{
	static_cast<QIODevice*>(stream)->write(data, len);
}

namespace Poppler {

class PSConverterData
{
	public:
		DocumentData *document;
		QString outputFileName;
		QIODevice *iodev;
		bool ownIodev;
		QList<int> pageList;
		QString title;
		double hDPI;
		double vDPI;
		int rotate;
		int paperWidth;
		int paperHeight;
		int marginRight;
		int marginBottom;
		int marginLeft;
		int marginTop;
		bool strictMargins;
		bool forceRasterize;
};

PSConverter::PSConverter(DocumentData *document)
{
	m_data = new PSConverterData();
	m_data->document = document;
	m_data->iodev = 0;
	m_data->ownIodev = true;
	m_data->hDPI = 72;
	m_data->vDPI = 72;
	m_data->rotate = 0;
	m_data->paperWidth = -1;
	m_data->paperHeight = -1;
	m_data->marginRight = 0;
	m_data->marginBottom = 0;
	m_data->marginLeft = 0;
	m_data->marginTop = 0;
	m_data->strictMargins = false;
	m_data->forceRasterize = false;
}

PSConverter::~PSConverter()
{
	delete m_data;
}

void PSConverter::setOutputFileName(const QString &outputFileName)
{
	m_data->outputFileName = outputFileName;
}

void PSConverter::setOutputDevice(QIODevice *device)
{
	m_data->iodev = device;
	m_data->ownIodev = false;
}

void PSConverter::setPageList(const QList<int> &pageList)
{
	m_data->pageList = pageList;
}

void PSConverter::setTitle(const QString &title)
{
	m_data->title = title;
}

void PSConverter::setHDPI(double hDPI)
{
	m_data->hDPI = hDPI;
}

void PSConverter::setVDPI(double vDPI)
{
	m_data->vDPI = vDPI;
}

void PSConverter::setRotate(int rotate)
{
	m_data->rotate = rotate;
}

void PSConverter::setPaperWidth(int paperWidth)
{
	m_data->paperWidth = paperWidth;
}

void PSConverter::setPaperHeight(int paperHeight)
{
	m_data->paperHeight = paperHeight;
}

void PSConverter::setRightMargin(int marginRight)
{
	m_data->marginRight = marginRight;
}

void PSConverter::setBottomMargin(int marginBottom)
{
	m_data->marginBottom = marginBottom;
}

void PSConverter::setLeftMargin(int marginLeft)
{
	m_data->marginLeft = marginLeft;
}

void PSConverter::setTopMargin(int marginTop)
{
	m_data->marginTop = marginTop;
}

void PSConverter::setStrictMargins(bool strictMargins)
{
	m_data->strictMargins = strictMargins;
}

void PSConverter::setForceRasterize(bool forceRasterize)
{
	m_data->forceRasterize = forceRasterize;
}

bool PSConverter::convert()
{
	if (!m_data->iodev)
	{
		Q_ASSERT(!m_data->outputFileName.isEmpty());
		QFile *f = new QFile(m_data->outputFileName);
		m_data->iodev = f;
		m_data->ownIodev = true;
	}
	Q_ASSERT(m_data->iodev);
	if (!m_data->iodev->isOpen())
	{
		if (!m_data->iodev->open(QIODevice::ReadWrite))
		{
			return false;
		}
	}

	Q_ASSERT(!m_data->pageList.isEmpty());
	Q_ASSERT(m_data->paperWidth != -1);
	Q_ASSERT(m_data->paperHeight != -1);
	
	QByteArray pstitle8Bit = m_data->title.toLocal8Bit();
	char* pstitlechar;
	if (!m_data->title.isEmpty()) pstitlechar = pstitle8Bit.data();
	else pstitlechar = 0;
	
	PSOutputDev *psOut = new PSOutputDev(outputToQIODevice, m_data->iodev,
	                                     pstitlechar,
	                                     m_data->document->doc->getXRef(),
	                                     m_data->document->doc->getCatalog(),
	                                     1,
	                                     m_data->document->doc->getNumPages(),
	                                     psModePS,
	                                     m_data->paperWidth,
	                                     m_data->paperHeight,
	                                     gFalse,
	                                     m_data->marginLeft,
	                                     m_data->marginBottom,
	                                     m_data->paperWidth - m_data->marginRight,
	                                     m_data->paperHeight - m_data->marginTop,
	                                     m_data->forceRasterize);
	
	if (m_data->strictMargins)
	{
		double xScale = ((double)m_data->paperWidth - (double)m_data->marginLeft - (double)m_data->marginRight) / (double)m_data->paperWidth;
		double yScale = ((double)m_data->paperHeight - (double)m_data->marginBottom - (double)m_data->marginTop) / (double)m_data->paperHeight;
		psOut->setScale(xScale, yScale);
	}
	
	if (psOut->isOk())
	{
		foreach(int page, m_data->pageList)
		{
			m_data->document->doc->displayPage(psOut, page, m_data->hDPI, m_data->vDPI, m_data->rotate, gFalse, gTrue, gFalse);
		}
		delete psOut;
		if (m_data->ownIodev)
		{
			m_data->iodev->close();
			delete m_data->iodev;
			m_data->iodev = 0;
		}
		return true;
	}
	else
	{
		delete psOut;
		if (m_data->ownIodev)
		{
			m_data->iodev->close();
			delete m_data->iodev;
			m_data->iodev = 0;
		}
		return false;
	}
}

}
