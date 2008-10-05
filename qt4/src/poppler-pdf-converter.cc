/* poppler-pdf-converter.cc: qt4 interface to poppler
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Albert Astals Cid <aacid@kde.org>
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
#include "poppler-converter-private.h"
#include "poppler-qiodeviceoutstream-private.h"

#include <QtCore/QFile>

namespace Poppler {

class PDFConverterPrivate : public BaseConverterPrivate
{
	public:
		PDFConverterPrivate();

		PDFConverter::PDFOptions opts;
};

PDFConverterPrivate::PDFConverterPrivate()
	: BaseConverterPrivate(), opts(0)
{
}


PDFConverter::PDFConverter(DocumentData *document)
	: BaseConverter(*new PDFConverterPrivate())
{
	Q_D(PDFConverter);
	d->document = document;
}

PDFConverter::~PDFConverter()
{
}

void PDFConverter::setPDFOptions(PDFConverter::PDFOptions options)
{
	Q_D(PDFConverter);
	d->opts = options;
}

PDFConverter::PDFOptions PDFConverter::pdfOptions() const
{
	Q_D(const PDFConverter);
	return d->opts;
}

bool PDFConverter::convert()
{
	Q_D(PDFConverter);

	if (d->document->locked)
		return false;

	QIODevice *dev = d->openDevice();
	if (!dev)
		return false;

	bool deleteFile = false;
	if (QFile *file = qobject_cast<QFile*>(dev))
		deleteFile = !file->exists();

	bool success;
	QIODeviceOutStream stream(dev);
	if (d->opts & WithChanges)
	{
		success = d->document->doc->saveAs(&stream);
	}
	else
	{
		success = d->document->doc->saveWithoutChangesAs(&stream);
	}
	d->closeDevice();
	if (!success)
	{
		if (deleteFile)
		{
			qobject_cast<QFile*>(dev)->remove();
		}
	}

	return success;
}

}
