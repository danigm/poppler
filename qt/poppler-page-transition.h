/* PageTransition.h
 * Copyright (C) 2005, Net Integration Technologies, Inc.
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

#ifndef __PAGETRANSITION_X_H__
#define __PAGETRANSITION_X_H__

namespace Poppler {

class PageTransitionParams;
class PageTransitionData;

class PageTransition {
 public:
  enum Type {
    Replace,
    Split,
    Blinds,
    Box,
    Wipe,
    Dissolve,
    Glitter,
    Fly,
    Push,
    Cover,
    Uncover,
    Fade
  };
  
  enum Alignment {
    Horizontal,
    Vertical
  };
  
  enum Direction {
    Inward,
    Outward
  };
  
  /** \brief Construct a new PageTransition object from a page dictionary.

  In case or error, this method will print an error message to stderr,
  and construct a default object.

  @param dictObj pointer to an object whose dictionary will be read
  and parsed. The pointer dictObj must point to a valid object, whose
  dictionaries are accessed by the constructor. The dictObj is only
  accessed by this constructor, and may be deleted after the
  constructor returns.
  */
  PageTransition(const PageTransitionParams &params);
  
  PageTransition(const PageTransition &pt);
  
  /**
     Destructor
  */
  ~PageTransition();
  
  /**
     \brief Get type of the transition.
  */
  Type type() const;
  
  /**
     \brief Get duration of the transition in seconds.
  */
  int duration() const;
  
  /**
     \brief Get dimension in which the transition effect occurs.
  */
  Alignment alignment() const;
  
  /**
     \brief Get direction of motion of the transition effect.
  */
  Direction direction() const;
  
  /**
     \brief Get direction in which the transition effect moves.
  */
  int angle() const;
  
  /**
     \brief Get starting or ending scale.
  */
  double scale() const;
  
  /**
     \brief Returns true if the area to be flown is rectangular and
     opaque.
  */
  bool isRectangular() const;
  
 private:
  PageTransitionData *data;
};

}

#endif
