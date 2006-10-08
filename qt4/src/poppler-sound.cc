/* poppler-sound.cc: qt interface to poppler
 * Copyright (C) 2006, Pino Toscano <pino@kde.org>
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

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "Object.h"
#include "Dict.h"
#include "Stream.h"
#include "Sound.h"

#include "poppler-qt4.h"

namespace Poppler
{

class SoundData
{
public:
	SoundData()
	  : m_channels( 1 ), m_bitsPerSample( 8 ), m_soundEncoding( SoundObject::Raw ), m_soundObj( new Object() )
	{ m_soundObj->initNull(); }

	QVariant m_data;
	SoundObject::SoundType m_type;
	double m_samplingRate;
	int m_channels;
	int m_bitsPerSample;
	SoundObject::SoundEncoding m_soundEncoding;
	Object *m_soundObj;
};

SoundObject::SoundObject(Sound *popplersound)
{
	m_soundData = new SoundData();
	Dict *dict = popplersound->getStream()->getDict();
	Object tmp;
	// file specs / data
	dict->lookup("F", &tmp);
	if ( !tmp.isNull() )
	{
		// valid 'F' key -> external file
		m_soundData->m_type = SoundObject::External;
		// TODO read the file specifications
		m_soundData->m_data = QVariant( QString() );
	}
	else
	{
		// no file specification, then the sound data have to be
		// extracted from the stream
		m_soundData->m_type = SoundObject::Embedded;
		Stream *stream = popplersound->getStream();
		stream->reset();
		int dataLen = 0;
		QByteArray fileArray;
		int i;
		while ( (i = stream->getChar()) != EOF) {
			fileArray[dataLen] = (char)i;
			++dataLen;
		}
		fileArray.resize(dataLen);
		m_soundData->m_data = QVariant( fileArray );
	}
	tmp.free();
	// sampling rate
	dict->lookup( "R", &tmp );
	if ( tmp.isNum() )
	{
		m_soundData->m_samplingRate = tmp.getNum();
	}
	tmp.free();
	// sound channels
	dict->lookup( "C", &tmp );
	if ( tmp.isInt() )
	{
		m_soundData->m_channels = tmp.getInt();
	}
	tmp.free();
	// sound channels
	dict->lookup( "B", &tmp );
	if ( tmp.isInt() )
	{
		m_soundData->m_bitsPerSample = tmp.getInt();
	}
	tmp.free();
	// encoding format
	dict->lookup( "E", &tmp );
	if ( tmp.isName() )
	{
		const char *enc = tmp.getName();
		if ( !strcmp( "Raw", enc ) )
			m_soundData->m_soundEncoding = SoundObject::Raw;
		else if ( !strcmp( "Signed", enc ) )
			m_soundData->m_soundEncoding = SoundObject::Signed;
		if ( !strcmp( "muLaw", enc ) )
			m_soundData->m_soundEncoding = SoundObject::muLaw;
		if ( !strcmp( "ALaw", enc ) )
			m_soundData->m_soundEncoding = SoundObject::ALaw;
	}
	tmp.free();
	// at the end, copying the object
	popplersound->getObject()->copy(m_soundData->m_soundObj);
}

SoundObject::SoundObject(const SoundObject &s)
{
	m_soundData = new SoundData();
	m_soundData->m_type = s.m_soundData->m_type;
	m_soundData->m_data = s.m_soundData->m_data;
	m_soundData->m_type = s.m_soundData->m_type;
	m_soundData->m_samplingRate = s.m_soundData->m_samplingRate;
	m_soundData->m_channels = s.m_soundData->m_channels;
	m_soundData->m_bitsPerSample = s.m_soundData->m_bitsPerSample;
	m_soundData->m_soundEncoding = s.m_soundData->m_soundEncoding;
	s.m_soundData->m_soundObj->copy(m_soundData->m_soundObj);
}

SoundObject::~SoundObject()
{
	m_soundData->m_soundObj->free();
	delete m_soundData;
}

SoundObject::SoundType SoundObject::soundType() const
{
	return m_soundData->m_type;
}

QString SoundObject::url() const
{
	return m_soundData->m_type == SoundObject::External ? m_soundData->m_data.toString() : QString();
}

QByteArray SoundObject::data() const
{
	return m_soundData->m_type == SoundObject::Embedded ? m_soundData->m_data.toByteArray() : QByteArray();
};

double SoundObject::samplingRate() const
{
	return m_soundData->m_samplingRate;
}

int SoundObject::channels() const
{
	return m_soundData->m_channels;
}

int SoundObject::bitsPerSample() const
{
	return m_soundData->m_bitsPerSample;
}

SoundObject::SoundEncoding SoundObject::soundEncoding() const
{
	return m_soundData->m_soundEncoding;
}

}
