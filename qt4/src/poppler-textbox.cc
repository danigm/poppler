/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
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

#include "poppler-qt4.h"
#include "poppler-private.h"

namespace Poppler {

TextBox::TextBox(const QString& text, const QRectF &bBox)
{
	m_data = new TextBoxData();
	m_data->text = text;
	m_data->bBox = bBox;
}

const QString &TextBox::text() const
{
	return m_data->text;
}

const QRectF &TextBox::boundingBox() const
{
	return m_data->bBox;
};

TextBox *TextBox::nextWord() const
{
	return m_data->nextWord;
}

double TextBox::edge(int i) const
{
	return m_data->edge[i];
}

bool TextBox::hasSpaceAfter() const
{
	return m_data->hasSpaceAfter;
}

}
