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

#include "permissions.h"

#include <poppler-qt4.h>

#include <QtGui/QTableWidget>

static QString yesNoStatement(bool value)
{
    return value ? QString::fromLatin1("yes") : QString::fromLatin1("no");
}

PermissionsDock::PermissionsDock(QWidget *parent)
    : AbstractInfoDock(parent)
{
    m_table = new QTableWidget(this);
    setWidget(m_table);
    setWindowTitle(tr("Permissions"));
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels(QStringList() << tr("Permission") << tr("Value"));
    m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

PermissionsDock::~PermissionsDock()
{
}

void PermissionsDock::fillInfo()
{
    m_table->setHorizontalHeaderLabels(QStringList() << tr("Permission") << tr("Value"));
    int i = 0;
#define ADD_ROW(title, function) \
do { \
    m_table->setRowCount(i + 1); \
    m_table->setItem(i, 0, new QTableWidgetItem(QString::fromLatin1(title))); \
    m_table->setItem(i, 1, new QTableWidgetItem(yesNoStatement(document()->function()))); \
    ++i; \
} while (0)
    ADD_ROW("Print", okToPrint);
    ADD_ROW("PrintHiRes", okToPrintHighRes);
    ADD_ROW("Change", okToChange);
    ADD_ROW("Copy", okToCopy);
    ADD_ROW("Add Notes", okToAddNotes);
    ADD_ROW("Fill Forms", okToFillForm);
    ADD_ROW("Create Forms", okToCreateFormFields);
    ADD_ROW("Extract for accessibility", okToExtractForAccessibility);
    ADD_ROW("Assemble", okToAssemble);
#undef ADD_ROW
}

void PermissionsDock::documentClosed()
{
    m_table->clear();
    m_table->setRowCount(0);
    AbstractInfoDock::documentClosed();
}

#include "permissions.moc"
