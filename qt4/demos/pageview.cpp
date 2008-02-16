/*
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
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

#include "pageview.h"

#include <poppler-qt4.h>

#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>

PageView::PageView(QWidget *parent)
    : QScrollArea(parent)
{
    m_imageLabel = new QLabel(this);
    m_imageLabel->resize(0, 0);
    setWidget(m_imageLabel);
}

PageView::~PageView()
{
}

void PageView::documentLoaded()
{
}

void PageView::documentClosed()
{
}

void PageView::pageChanged(int page)
{
    Poppler::Page *popplerPage = document()->page(page);
    QImage image = popplerPage->renderToImage();
    if (!image.isNull()) {
        m_imageLabel->resize(image.size());
        m_imageLabel->setPixmap(QPixmap::fromImage(image));
    } else {
        m_imageLabel->resize(0, 0);
        m_imageLabel->setPixmap(QPixmap());
    }
    delete popplerPage;
}

#include "pageview.moc"
