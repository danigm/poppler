/*
 * Copyright (C) 2009, Pino Toscano <pino@kde.org>
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

#include "poppler-document.h"
#include "poppler-embedded-file.h"
#include "poppler-page.h"
#include "poppler-toc.h"

#include "poppler-document-private.h"
#include "poppler-embedded-file-private.h"
#include "poppler-private.h"
#include "poppler-toc-private.h"

#include "Catalog.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include "Outline.h"

#include <algorithm>
#include <memory>

using namespace poppler;

unsigned int poppler::document_private::count = 0U;

document_private::document_private(GooString *file_path, const std::string &owner_password,
                                   const std::string &user_password)
    : doc(0)
    , is_locked(false)
{
    GooString goo_owner_password(owner_password.c_str());
    GooString goo_user_password(user_password.c_str());
    doc = new PDFDoc(file_path, &goo_owner_password, &goo_user_password);
    init();
}

document_private::document_private(byte_array *file_data,
                                   const std::string &owner_password,
                                   const std::string &user_password)
    : doc(0)
    , is_locked(false)
{
    Object obj;
    obj.initNull();
    file_data->swap(doc_data);
    MemStream *memstr = new MemStream(&doc_data[0], 0, doc_data.size(), &obj);
    GooString goo_owner_password(owner_password.c_str());
    GooString goo_user_password(user_password.c_str());
    doc = new PDFDoc(memstr, &goo_owner_password, &goo_user_password);
    init();
}

document_private::~document_private()
{
    delete_all(embedded_files);

    delete doc;

    if (count > 0) {
        --count;
        if (!count) {
            delete globalParams;
            globalParams = 0;
        }
    }
}

void document_private::init()
{
    if (!count) {
        globalParams = new GlobalParams();
        setErrorFunction(detail::error_function);
    }
    count++;
}

document* document_private::check_document(document_private *doc)
{
    if (doc->doc->isOk() || doc->doc->getErrorCode() == errEncrypted) {
        if (doc->doc->getErrorCode() == errEncrypted) {
            doc->is_locked = true;
        }
        return new document(*doc);
    } else {
        delete doc;
    }
    return 0;
}


document::document(document_private &dd)
    : d(&dd)
{
}

document::~document()
{
    delete d;
}

bool document::is_locked() const
{
    return d->is_locked;
}

bool document::unlock(const std::string &owner_password, const std::string &user_password)
{
    if (d->is_locked) {
        document_private *newdoc = 0;
        if (d->doc_data.size() > 0) {
            newdoc = new document_private(&d->doc_data,
                                          owner_password, user_password);
        } else {
            newdoc = new document_private(new GooString(d->doc->getFileName()),
                                          owner_password, user_password);
        }
        if (!newdoc->doc->isOk()) {
            d->doc_data.swap(newdoc->doc_data);
            delete newdoc;
        } else {
            delete d;
            d = newdoc;
            d->is_locked = false;
        }
    }
    return d->is_locked;
}

document::page_mode_enum document::page_mode() const
{
    switch (d->doc->getCatalog()->getPageMode()) {
    case Catalog::pageModeNone:
        return use_none;
    case Catalog::pageModeOutlines:
        return use_outlines;
    case Catalog::pageModeThumbs:
        return use_thumbs;
    case Catalog::pageModeFullScreen:
        return fullscreen;
    case Catalog::pageModeOC:
        return use_oc;
    case Catalog::pageModeAttach:
        return use_attach;
    default:
        return use_none;
    }
}

document::page_layout_enum document::page_layout() const
{
    switch (d->doc->getCatalog()->getPageLayout()) {
    case Catalog::pageLayoutNone:
        return no_layout;
    case Catalog::pageLayoutSinglePage:
        return single_page;
    case Catalog::pageLayoutOneColumn:
        return one_column;
    case Catalog::pageLayoutTwoColumnLeft:
        return two_column_left;
    case Catalog::pageLayoutTwoColumnRight:
        return two_column_right;
    case Catalog::pageLayoutTwoPageLeft:
        return two_page_left;
    case Catalog::pageLayoutTwoPageRight:
        return two_page_right;
    default:
        return no_layout;
    }
}

void document::get_pdf_version(int *major, int *minor) const
{
    if (major) {
        *major = d->doc->getPDFMajorVersion();
    }
    if (minor) {
        *minor = d->doc->getPDFMinorVersion();
    }
}

std::vector<std::string> document::info_keys() const
{
    if (d->is_locked) {
        return std::vector<std::string>();
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return std::vector<std::string>();
    }

    Dict *info_dict = info.getDict();
    std::vector<std::string> keys(info_dict->getLength());
    for (int i = 0; i < info_dict->getLength(); ++i) {
        keys[i] = std::string(info_dict->getKey(i));
    }

    info.free();
    return keys;
}

ustring document::info_key(const std::string &key) const
{
    if (d->is_locked) {
        return ustring();
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return ustring();
    }

    Dict *info_dict = info.getDict();
    Object obj;
    ustring result;
    if (info_dict->lookup(PSTR(key.c_str()), &obj)->isString()) {
        result = detail::unicode_GooString_to_ustring(obj.getString());
    }
    obj.free();
    info.free();
    return result;
}

unsigned int document::info_date(const std::string &key) const
{
    if (d->is_locked) {
        return (unsigned int)(-1);
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return (unsigned int)(-1);
    }

    Dict *info_dict = info.getDict();
    Object obj;
    unsigned int result = (unsigned int)(-1);
    if (info_dict->lookup(PSTR(key.c_str()), &obj)->isString()) {
        result = convert_date(obj.getString()->getCString());
    }
    obj.free();
    info.free();
    return result;
}

bool document::is_encrypted() const
{
    return d->doc->isEncrypted();
}

bool document::is_linearized() const
{
    return d->doc->isLinearized();
}

bool document::has_permission(permission_enum which) const
{
    switch (which) {
    case perm_print:
        return d->doc->okToPrint();
    case perm_change:
        return d->doc->okToChange();
    case perm_copy:
        return d->doc->okToCopy();
    case perm_add_notes:
        return d->doc->okToAddNotes();
    case perm_fill_forms:
        return d->doc->okToFillForm();
    case perm_accessibility:
        return d->doc->okToAccessibility();
    case perm_assemble:
        return d->doc->okToAssemble();
    case perm_print_high_resolution:
        return d->doc->okToPrintHighRes();
    }
    return true;
}

ustring document::metadata() const
{
    std::auto_ptr<GooString> md(d->doc->getCatalog()->readMetadata());
    if (md.get()) {
        return detail::unicode_GooString_to_ustring(md.get());
    }
    return ustring();
}

int document::pages() const
{
    return d->doc->getNumPages();
}

page* document::create_page(const ustring &label) const
{
    std::auto_ptr<GooString> goolabel(detail::ustring_to_unicode_GooString(label));
    int index = 0;

    if (!d->doc->getCatalog()->labelToIndex(goolabel.get(), &index)) {
        return 0;
    }
    return create_page(index);
}

page* document::create_page(int index) const
{
    return index >= 0 && index < d->doc->getNumPages() ? new page(d, index) : 0;
}

std::vector<font_info> document::fonts() const
{
    std::vector<font_info> result;
    font_iterator it(0, d);
    while (it.has_next()) {
        const std::vector<font_info> l = it.next();
        std::copy(l.begin(), l.end(), std::back_inserter(result));
    }
    return result;
}

font_iterator* document::create_font_iterator(int start_page) const
{
    return new font_iterator(start_page, d);
}

toc* document::create_toc() const
{
    return toc_private::load_from_outline(d->doc->getOutline());
}

bool document::has_embedded_files() const
{
    return d->doc->getCatalog()->numEmbeddedFiles() > 0;
}

std::vector<embedded_file *> document::embedded_files() const
{
    if (d->is_locked) {
        return std::vector<embedded_file *>();
    }

    if (d->embedded_files.empty() && d->doc->getCatalog()->numEmbeddedFiles() > 0) {
        const int num = d->doc->getCatalog()->numEmbeddedFiles();
        d->embedded_files.resize(num);
        for (int i = 0; i < num; ++i) {
            EmbFile *ef = d->doc->getCatalog()->embeddedFile(i);
            d->embedded_files[i] = embedded_file_private::create(ef);
        }
    }
    return d->embedded_files;
}

document* document::load_from_file(const std::string &file_name,
                                   const std::string &owner_password,
                                   const std::string &user_password)
{
    document_private *doc = new document_private(
                                new GooString(file_name.c_str()),
                                owner_password, user_password);
    return document_private::check_document(doc);
}

document* document::load_from_data(byte_array *file_data,
                                   const std::string &owner_password,
                                   const std::string &user_password)
{
    if (!file_data || file_data->size() < 10) {
        return 0;
    }

    document_private *doc = new document_private(
                                file_data, owner_password, user_password);
    return document_private::check_document(doc);
}
