/* Sound.h - an object that holds the sound structure
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

#ifndef Sound_H
#define Sound_H

class Object;
class Stream;

//------------------------------------------------------------------------

class Sound
{
public:
  // Try to parse the Object s
  static Sound *parseSound(Object *s);

  // Destructor
  ~Sound();

  Object *getObject() { return streamObj; }
  Stream *getStream();

private:
  // Create a sound. The Object obj is ensured to be a Stream with a Dict
  Sound(Object *obj);

  Object *streamObj;
};

#endif
