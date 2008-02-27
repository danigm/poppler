/* poppler-optcontent-private.h: qt interface to poppler
 *
 * Copyright (C) 2007, Brad Hards <bradh@kde.org>
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

#ifndef POPPLER_OPTCONTENT_PRIVATE_H
#define POPPLER_OPTCONTENT_PRIVATE_H

#include "poppler-optcontent.h"

#include "goo/GooList.h"

namespace Poppler
{

  class RadioButtonGroup
  {
  public:
    RadioButtonGroup( OptContentModelPrivate *ocModel, Array *rbarray);
    ~RadioButtonGroup();
    void setItemOn( OptContentItem *itemToSetOn );

  private:
    QList<OptContentItem*> itemsInGroup;
  };

  class OptContentModelPrivate
  {
    public:
    OptContentModelPrivate( OptContentModel *qq, OCGs *optContent );
    ~OptContentModelPrivate();

    void parseRBGroupsArray( Array *rBGroupArray );
    OptContentItem *nodeFromIndex( const QModelIndex &index ) const;

    OptContentModel *q;

    QMap<QString, OptContentItem*> m_optContentItems;
    QList<RadioButtonGroup*> m_rbgroups;
    OptContentItem *m_rootNode;

    private:
    void addChild( OptContentItem *parent, OptContentItem *child);
    void parseOrderArray( OptContentItem *parentNode, Array *orderArray );
  };
}

#endif
