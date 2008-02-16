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

#include "viewer.h"

#include "navigationtoolbar.h"
#include "pageview.h"

#include <poppler-qt4.h>

#include <QtCore/QDir>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>

PdfViewer::PdfViewer()
    : QMainWindow(), m_doc(0)
{
    // setup the menus
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileOpenAct = fileMenu->addAction(tr("&Open"), this, SLOT(slotOpenFile()));
    m_fileOpenAct->setShortcut(Qt::CTRL + Qt::Key_O);
    QAction *act = fileMenu->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()));
    act->setShortcut(Qt::CTRL + Qt::Key_Q);

    NavigationToolBar *navbar = new NavigationToolBar(this);
    addToolBar(navbar);
    m_observers.append(navbar);

    PageView *view = new PageView(this);
    setCentralWidget(view);
    m_observers.append(view);

    Q_FOREACH(DocumentObserver *obs, m_observers) {
        obs->m_viewer = this;
    }
}

PdfViewer::~PdfViewer()
{
    closeDocument();
}

QSize PdfViewer::sizeHint() const
{
    return QSize(500, 600);
}

void PdfViewer::loadDocument(const QString &file)
{
    Poppler::Document *newdoc = Poppler::Document::load(file);
    if (!newdoc) {
        QMessageBox msgbox(QMessageBox::Critical, tr("Open Error"), tr("Cannot open:\n") + file,
                           QMessageBox::Ok, this);
        msgbox.exec();
        return;
    }

    while (newdoc->isLocked()) {
        bool ok = true;
        QString password = QInputDialog::getText(this, tr("Document Password"),
                                                 tr("Please insert the password of the document:"),
                                                 QLineEdit::Password, QString(), &ok);
        if (!ok) {
            delete newdoc;
            return;
        }
        newdoc->unlock(password.toLatin1(), password.toLatin1());
    }

    closeDocument();

    m_doc = newdoc;

    Q_FOREACH(DocumentObserver *obs, m_observers) {
        obs->documentLoaded();
        obs->pageChanged(0);
    }
}

void PdfViewer::closeDocument()
{
    if (!m_doc) {
        return;
    }

    Q_FOREACH(DocumentObserver *obs, m_observers) {
        obs->documentClosed();
    }

    delete m_doc;
    m_doc = 0;
}

void PdfViewer::slotOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open PDF Document"), QDir::homePath(), tr("PDF Documents (*.pdf)"));
    if (fileName.isEmpty()) {
        return;
    }

    loadDocument(fileName);
}

void PdfViewer::setPage(int page)
{
    Q_FOREACH(DocumentObserver *obs, m_observers) {
        obs->pageChanged(page);
    }

    m_currentPage = page;
}

int PdfViewer::page() const
{
    return m_currentPage;
}

#include "viewer.moc"
