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

#include "poppler-embedded-file.h"

#include "poppler-embedded-file-private.h"
#include "poppler-private.h"

#include "Object.h"
#include "Stream.h"
#include "Catalog.h"

using namespace poppler;

embedded_file_private::embedded_file_private(EmbFile *ef)
    : emb_file(ef)
{
}

embedded_file_private::~embedded_file_private()
{
    delete emb_file;
}

embedded_file* embedded_file_private::create(EmbFile *ef)
{
    return new embedded_file(*new embedded_file_private(ef));
}


embedded_file::embedded_file(embedded_file_private &dd)
    : d(&dd)
{
}

embedded_file::~embedded_file()
{
    delete d;
}

bool embedded_file::is_valid() const
{
    return d->emb_file->isOk();
}

std::string embedded_file::name() const
{
    return std::string(d->emb_file->name()->getCString());
}

ustring embedded_file::description() const
{
    return detail::unicode_GooString_to_ustring(d->emb_file->description());
}

int embedded_file::size() const
{
    return d->emb_file->size();
}

unsigned int embedded_file::modification_date() const
{
    return convert_date(d->emb_file->modDate()->getCString());
}

unsigned int embedded_file::creation_date() const
{
    return convert_date(d->emb_file->createDate()->getCString());
}

std::string embedded_file::checksum() const
{
    return std::string(d->emb_file->checksum()->getCString());
}

std::string embedded_file::mime_type() const
{
    return std::string(d->emb_file->mimeType()->getCString());
}

std::vector<char> embedded_file::data() const
{
    if (!is_valid()) {
        return std::vector<char>();
    }

    Stream *stream = d->emb_file->streamObject().getStream();
    stream->reset();
    std::vector<char> ret(1024);
    size_t data_len = 0;
    int i;
    while ((i = stream->getChar()) != EOF) {
        if (data_len == ret.size()) {
            ret.resize(ret.size() * 2);
        }
        ret[data_len] = (char)i;
        ++data_len;
    }
    ret.resize(data_len);
    return ret;
}
