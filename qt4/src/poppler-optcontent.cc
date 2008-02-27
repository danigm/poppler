/* poppler-optcontent.cc: qt interface to poppler
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

#include "poppler-optcontent.h"

#include "poppler/OptionalContent.h"

#include "poppler-qt4.h"
#include "poppler-private.h"

#include <QtCore/QDebug>

namespace Poppler
{

  RadioButtonGroup::RadioButtonGroup( OptContentModel *ocModel, Array *rbarray )
  {
    for (int i = 0; i < rbarray->getLength(); ++i) {
      Object ref;
      rbarray->getNF( i, &ref );
      if ( ! ref.isRef() ) {
	qDebug() << "expected ref, but got:" << ref.getType();
      }
      OptContentItem *item = ocModel->itemFromRef( QString::number(ref.getRefNum() ) );
      itemsInGroup.append( item );
    }
    for (int i = 0; i < itemsInGroup.size(); ++i) {
      OptContentItem *item = itemsInGroup.at(i);
      item->appendRBGroup( this );
    }
  }

  RadioButtonGroup::~RadioButtonGroup()
  {
  }

  void RadioButtonGroup::setItemOn( OptContentItem *itemToSetOn )
  {
    for (int i = 0; i < itemsInGroup.size(); ++i) {
      OptContentItem *thisItem = itemsInGroup.at(i);
      if (thisItem != itemToSetOn) {
	thisItem->setState( OptContentItem::Off );
      }
    }
  }



  OptContentItem::OptContentItem( OptionalContentGroup *group )
  {
    m_group = group;
    m_parent = 0;
    m_name = UnicodeParsedString( group->name() );
    if ( group->state() == OptionalContentGroup::On ) {
      m_state = OptContentItem::On;
    } else {
      m_state = OptContentItem::Off;
    }
  }

  OptContentItem::OptContentItem( const QString &label )
  {
    m_parent = 0;
    m_name = label;
    m_group = 0;
    m_state = OptContentItem::HeadingOnly;
  }

  OptContentItem::OptContentItem()
  {
    m_parent = 0;
  }

  OptContentItem::~OptContentItem()
  {
  }

  void OptContentItem::appendRBGroup( RadioButtonGroup *rbgroup )
  {
    m_rbGroups.append( rbgroup );
  }


  bool OptContentItem::setState( ItemState state )
  {
    m_state = state;
    if (!m_group) {
      return false;
    }
    if ( state == OptContentItem::On ) {
      m_group->setState( OptionalContentGroup::On );
      for (int i = 0; i < m_rbGroups.size(); ++i) {
	RadioButtonGroup *rbgroup = m_rbGroups.at(i);
	rbgroup->setItemOn( this );
      }
    } else if ( state == OptContentItem::Off ) {
      m_group->setState( OptionalContentGroup::Off );
    }
    return true;
  }

  void OptContentItem::addChild( OptContentItem *child )
  {
    m_children += child;
    child->setParent( this );
  }

  OptContentModel::OptContentModel( OCGs *optContent, QObject *parent)
    : QAbstractItemModel(parent)
  {
    m_rootNode = new OptContentItem();
    GooList *ocgs = optContent->getOCGs();

    for (int i = 0; i < ocgs->getLength(); ++i) {
      OptionalContentGroup *ocg = static_cast<OptionalContentGroup*>(ocgs->get(i));
      OptContentItem *node = new OptContentItem( ocg );
      m_optContentItems.insert( QString::number(ocg->ref().num), node);
    }

    if ( optContent->getOrderArray() == 0 ) {
      // no Order array, so drop them all at the top level
      QMapIterator<QString, OptContentItem*> i(m_optContentItems);
      while ( i.hasNext() ) {
	i.next();
	qDebug() << "iterator" << i.key() << ":" << i.value();
	addChild( i.value(), m_rootNode );
      }
    } else {
      parseOrderArray( m_rootNode, optContent->getOrderArray() );
    }

    parseRBGroupsArray( optContent->getRBGroupsArray() );
  }

  OptContentModel::~OptContentModel()
  {
    qDeleteAll( m_optContentItems );
    qDeleteAll( m_rbgroups );
    delete m_rootNode;
  }

  void OptContentModel::setRootNode( OptContentItem *node )
  {
    delete m_rootNode;
    m_rootNode = node;
    reset();
  }

  void OptContentModel::parseOrderArray( OptContentItem *parentNode, Array *orderArray )
  {
    OptContentItem *lastItem = parentNode;
    for (int i = 0; i < orderArray->getLength(); ++i) {
      Object orderItem;
      orderArray->get(i, &orderItem);
      if ( orderItem.isDict() ) {
	Object item;
	orderArray->getNF(i, &item);
	if (item.isRef() ) {
	  OptContentItem *ocItem = m_optContentItems[ QString("%1").arg(item.getRefNum()) ];
	  if (ocItem) {
	    addChild( parentNode, ocItem );
	    lastItem = ocItem;
	  } else {
	    printf("couldn't find group for object %i\n", item.getRefNum());
	  }
	}
	item.free();
      } else if ( (orderItem.isArray()) && (orderItem.arrayGetLength() > 0) ) {
	parseOrderArray(lastItem, orderItem.getArray());
      } else if ( orderItem.isString() ) {
	GooString *label = orderItem.getString();
	OptContentItem *header = new OptContentItem ( UnicodeParsedString ( label ) );
	addChild( parentNode, header );
	parentNode = header;
	lastItem = header;
      } else {
	qDebug() << "something unexpected";
      }	
      orderItem.free();
    }
  }

  void OptContentModel::parseRBGroupsArray( Array *rBGroupArray )
  {
    if (! rBGroupArray) {
      return;
    }
    // This is an array of array(s)
    for (int i = 0; i < rBGroupArray->getLength(); ++i) {
      Object rbObj;
      rBGroupArray->get(i, &rbObj);
      if ( ! rbObj.isArray() ) {
	qDebug() << "expected inner array, got:" << rbObj.getType();
	return;
      }
      Array *rbarray = rbObj.getArray();
      RadioButtonGroup *rbg = new RadioButtonGroup( this, rbarray );
      m_rbgroups.append( rbg );
      rbObj.free();
    }
  }

  QModelIndex OptContentModel::index(int row, int column, const QModelIndex &parent) const
  {
    if (!m_rootNode) {
      return QModelIndex();
    }

    OptContentItem *parentNode = nodeFromIndex( parent );
    return createIndex( row, column, parentNode->childList()[row] );
  }

  QModelIndex OptContentModel::parent(const QModelIndex &child) const
  {
    OptContentItem *childNode = nodeFromIndex( child );
    if (!childNode) {
      return QModelIndex();
    }
    OptContentItem *parentNode = childNode->parent();
    if (!parentNode) {
      return QModelIndex();
    }
    OptContentItem *grandparentNode = parentNode->parent();
    if (!grandparentNode) {
      return QModelIndex();
    }
    int row = grandparentNode->childList().indexOf(parentNode);
    return createIndex(row, child.column(), parentNode);
  }
 
  int OptContentModel::rowCount(const QModelIndex &parent) const
  {
    OptContentItem *parentNode = nodeFromIndex( parent );
    if (!parentNode) {
      return 0;
    } else {
      return parentNode->childList().count();
    }
  }

  int OptContentModel::columnCount(const QModelIndex &parent) const
  {
    return 2;
  }


  QVariant OptContentModel::data(const QModelIndex &index, int role) const
  {
    if ( (role != Qt::DisplayRole) && (role != Qt::EditRole) ) {
      return QVariant();
    }

    OptContentItem *node = nodeFromIndex( index );
    if (!node) {
      return QVariant();
    }

    if (index.column() == 0) {
      return node->name();
    } else if (index.column() == 1) {
      if ( node->state() == OptContentItem::On ) {
	return true;
      } else if ( node->state() == OptContentItem::Off ) {
	return false;
      } else {
	return QVariant();
      }
    }

    return QVariant();
  }

  bool OptContentModel::setData ( const QModelIndex & index, const QVariant & value, int role )
  {
    OptContentItem *node = nodeFromIndex( index );
    if (!node) {
      return false;
    }

    if (index.column() == 0) {
      // we don't allow setting of the label
      return false;
    } else if (index.column() == 1) {
      if ( value.toBool() == true ) {
	if ( node->state() != OptContentItem::On ) {
	  node->setState( OptContentItem::On );
	  emit dataChanged( index, index );
	}
	return true;
      } else if ( value.toBool() == false ) {
	if ( node->state() != OptContentItem::Off ) {
	  node->setState( OptContentItem::Off );
	  emit dataChanged( index, index );
	}
	return true;
      } else {
	return false;
      }
    }

    return false;
  }

  Qt::ItemFlags OptContentModel::flags ( const QModelIndex & index ) const
  {
    if (index.column() == 0) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else if (index.column() == 1) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    } else {
      return QAbstractItemModel::flags(index);
    }
  }

  void OptContentModel::addChild( OptContentItem *parent, OptContentItem *child )
  {
    parent->addChild( child );
  }

  OptContentItem* OptContentModel::itemFromRef( const QString &ref ) const
  {
    if ( ! m_optContentItems.contains( ref ) ) {
      return 0;
    }
    return m_optContentItems[ ref ];
  }

  OptContentItem* OptContentModel::nodeFromIndex( const QModelIndex &index ) const
  {
    if (index.isValid()) {
      return static_cast<OptContentItem *>(index.internalPointer());
    } else {
      return m_rootNode;
    }
  }
}

#include "poppler-optcontent.moc"
