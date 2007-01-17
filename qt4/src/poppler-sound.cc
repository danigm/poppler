/* poppler-sound.cc: qt interface to poppler
 * Copyright (C) 2006-2007, Pino Toscano <pino@kde.org>
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

#include "Object.h"
#include "Stream.h"
#include "Sound.h"

#include "poppler-qt4.h"

namespace Poppler
{

class SoundData
{
public:
	SoundData()
	  : m_soundObj( 0 )
	{
	}

	~SoundData()
	{
		delete m_soundObj;
	}

	SoundObject::SoundType m_type;
	Sound *m_soundObj;
};

SoundObject::SoundObject(Sound *popplersound)
{
	m_soundData = new SoundData();
	switch ( popplersound->getSoundKind() )
	{
		case soundEmbedded:
			m_soundData->m_type = SoundObject::Embedded;
			break;
		case soundExternal:
		default:
			m_soundData->m_type = SoundObject::External;
			break;
	}

	m_soundData->m_soundObj = popplersound->copy();
}

SoundObject::SoundObject(const SoundObject &s)
{
	m_soundData = new SoundData();
	m_soundData->m_type = s.m_soundData->m_type;
	m_soundData->m_soundObj = s.m_soundData->m_soundObj->copy();
}

SoundObject::~SoundObject()
{
	delete m_soundData;
}

SoundObject::SoundType SoundObject::soundType() const
{
	return m_soundData->m_type;
}

QString SoundObject::url() const
{
	if ( m_soundData->m_type != SoundObject::External )
		return QString();

	GooString * goo = m_soundData->m_soundObj->getFileName();
	return goo ? QString( goo->getCString() ) : QString();
}

QByteArray SoundObject::data() const
{
	if ( m_soundData->m_type != SoundObject::Embedded )
		return QByteArray();

	Stream *stream = m_soundData->m_soundObj->getStream();
	stream->reset();
	int dataLen = 0;
	QByteArray fileArray;
	int i;
	while ( (i = stream->getChar()) != EOF) {
		fileArray[dataLen] = (char)i;
		++dataLen;
	}
	fileArray.resize(dataLen);

	return fileArray;
};

double SoundObject::samplingRate() const
{
	return m_soundData->m_soundObj->getSamplingRate();
}

int SoundObject::channels() const
{
	return m_soundData->m_soundObj->getChannels();
}

int SoundObject::bitsPerSample() const
{
	return m_soundData->m_soundObj->getBitsPerSample();
}

SoundObject::SoundEncoding SoundObject::soundEncoding() const
{
	switch ( m_soundData->m_soundObj->getEncoding() )
	{
		case soundRaw:
			return SoundObject::Raw;
		case soundSigned:
			return SoundObject::Signed;
		case soundMuLaw:
			return SoundObject::muLaw;
		case soundALaw:
			return SoundObject::ALaw;
	}
	return SoundObject::Raw;
}

}
