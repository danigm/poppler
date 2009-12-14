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

#include "poppler-global.h"

#include "DateInfo.h"

#include <ctime>
#include <cstring>
#include <iostream>

using namespace poppler;

noncopyable::noncopyable()
{
}

noncopyable::~noncopyable()
{
}


ustring::ustring()
{
}

ustring::ustring(size_type len, value_type ch)
    : std::basic_string<value_type>(len, ch)
{
}

ustring::~ustring()
{
}

byte_array ustring::to_utf_8() const
{
    if (!size()) {
        return byte_array();
    }

    const value_type *me = data();
    const size_t len = size() * 2 + 2;
    byte_array str(len);
    str[0] = 0xfe;
    str[1] = 0xff;
    for (size_t i = 0; i < size(); ++i, ++me) {
        str[i * 2 + 2] = (*me & 0xff);
        str[i * 2 + 3] = ((*me >> 8) & 0xff);
    }
    return str;
}

std::string ustring::to_latin1() const
{
    if (!size()) {
        return std::string();
    }

    const size_type mylength = size();
    std::string ret(mylength, '\0');
    const value_type *me = data();
    for (size_type i = 0; i < mylength; ++i) {
        ret[i] = (char)*me++;
    }
    return ret;
}

ustring ustring::from_utf_8(const char *str, int len)
{
    if (len <= 0) {
        len = std::strlen(str);
        if (len <= 0) {
            return ustring();
        }
    }

    int i = 0;
    bool is_unicode = false;
    if ((str[0] & 0xff) == 0xfe && (len > 1 && (str[1] & 0xff) == 0xff)) {
        is_unicode = true;
        i = 2;
    }

    const ustring::size_type ret_len = (len - i) / (is_unicode ? 2 : 1);
    ustring ret(ret_len, 0);
    size_t ret_index = 0;
    ustring::value_type u;
    if (is_unicode) {
        while (i < len) {
            u = ((str[i] & 0xff) << 8) | (str[i + 1] & 0xff);
            i += 2;
            ret[ret_index++] = u;
        }
    } else {
        while (i < len) {
            u = str[i] & 0xff;
            ++i;
            ret[ret_index++] = u;
        }
    }

    return ret;
}

ustring ustring::from_latin1(const std::string &str)
{
    const size_type l = str.size();
    if (!l) {
        return ustring();
    }
    const char *c = str.data();
    ustring ret(l, 0);
    for (size_type i = 0; i < l; ++i) {
        ret[i] = *c++;
    }
    return ret;
}


unsigned int convert_date(const std::string &date)
{
    int year, mon, day, hour, min, sec, tzHours, tzMins;
    char tz;

    if (!parseDateString(date.c_str(), &year, &mon, &day, &hour, &min, &sec,
                                       &tz, &tzHours, &tzMins)) {
        return (unsigned int)(-1);
    }

    struct tm time;
    time.tm_sec = sec;
    time.tm_min = min;
    time.tm_hour = hour;
    time.tm_mday = day;
    time.tm_mon = mon - 1;
    time.tm_year = year - 1900;
    time.tm_wday = -1;
    time.tm_yday = -1;
    time.tm_isdst = -1;
    return mktime(&time);
}

std::ostream& operator<<(std::ostream& stream, const byte_array &array)
{
    stream << "[";
    const std::ios_base::fmtflags f = stream.flags();
    std::hex(stream);
    const char *data = array.data();
    const byte_array::size_type out_len = std::min<byte_array::size_type>(array.size(), 50);
    for (byte_array::size_type i = 0; i < out_len; ++i)
    {
        if (i != 0) {
            stream << " ";
        }
        stream << ((data[i] & 0xf0) >> 4) << (data[i] & 0xf);
    }
    stream.flags(f);
    if (out_len < array.size()) {
        stream << " ...";
    }
    stream << "]";
    return stream;
}
