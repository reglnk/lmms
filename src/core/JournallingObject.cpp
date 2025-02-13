/*
 * JournallingObject.cpp - implementation of journalling-object related stuff
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include <QDomElement>

#include "JournallingObject.h"
#include "AutomatableModel.h"
#include "ProjectJournal.h"
#include "Engine.h"

namespace lmms
{


JournallingObject::JournallingObject() :
	SerializingObject(),
	m_id( Engine::projectJournal()->allocID( this ) ),
	m_journalling( true ),
	m_journallingStateStack()
{
}




JournallingObject::~JournallingObject()
{
	if( Engine::projectJournal() )
	{
		Engine::projectJournal()->freeID( id() );
	}
}




void JournallingObject::addJournalCheckPoint( int actionID )
{
	if( isJournalling() )
	{
		Engine::projectJournal()->addJournalCheckPoint( this, actionID );
	}
}




QDomElement JournallingObject::saveState( QDomDocument & _doc,
							QDomElement & _parent )
{
	if( isJournalling() )
	{
		QDomElement _this = SerializingObject::saveState( _doc, _parent );

		QDomElement journalNode = _doc.createElement( "journallingObject" );
		journalNode.setAttribute( "id", id() );
		journalNode.setAttribute( "metadata", true );
		_this.appendChild( journalNode );

		return _this;
	} else {
		return QDomElement();
	}
}




void JournallingObject::restoreState( const QDomElement & _this )
{
	SerializingObject::restoreState( _this );

	saveJournallingState( false );

	// search for journal-node
	QDomNode node = _this.firstChild();
	while( !node.isNull() )
	{
		if( node.isElement() && node.nodeName() == "journal" )
		{
			const jo_id_t new_id = node.toElement().attribute( "id" ).toInt();
			if( new_id )
			{
				changeID( new_id );
			}
		}
		node = node.nextSibling();
	}

	restoreJournallingState();
}




void JournallingObject::changeID( jo_id_t _id )
{
	if( id() != _id )
	{
		JournallingObject * jo = Engine::projectJournal()->
											journallingObject( _id );
		if( jo != nullptr )
		{
			QString used_by = jo->nodeName();
			if( used_by == "automatablemodel" &&
				dynamic_cast<AutomatableModel *>( jo ) )
			{
				used_by += ":" +
					dynamic_cast<AutomatableModel *>( jo )->
								displayName();
			}
			fprintf( stderr, "JO-ID %d already in use by %s!\n",
				(int) _id, used_by.toUtf8().constData() );
			return;
		}

		Engine::projectJournal()->freeID( m_id );
		Engine::projectJournal()->reallocID( _id, this );
		m_id = _id;
	}
}


} // namespace lmms
