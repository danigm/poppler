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

#include "poppler-version.h"

using namespace poppler;

std::string poppler::version_string()
{
    return std::string(POPPLER_VERSION);
}

unsigned int poppler::version_major()
{
    return POPPLER_VERSION_MAJOR;
}

unsigned int poppler::version_minor()
{
    return POPPLER_VERSION_MINOR;
}

unsigned int poppler::version_micro()
{
    return POPPLER_VERSION_MICRO;
}
