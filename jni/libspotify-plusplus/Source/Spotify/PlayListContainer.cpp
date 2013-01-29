/*
 * Copyright 2011 Jim Knowler
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#pragma once

// c-lib includes
#include <assert.h>

// local includes
#include "Spotify/PlayListContainer.h"
#include "Spotify/PlayListFolder.h"
#include "Spotify/Session.h"

// debugging
//#include "Debug/Debug.h"
#define LOG( msg, ... )	//Debug::PrintLine( msg, __VA_ARGS__ );

namespace Spotify
{
	PlayListContainer::PlayListContainer( Session* pSession ) : PlayListElement( pSession ), m_pContainer(NULL), m_isLoading(false)
	{
	}

	PlayListContainer::~PlayListContainer()
	{
		Unload();
	}

	PlayListElement::eType PlayListContainer::GetType()
	{
		return PLAYLIST_CONTAINER;
	}

	void PlayListContainer::GetCallbacks( sp_playlistcontainer_callbacks& callbacks )
	{
		memset( &callbacks, 0, sizeof(callbacks) );

		callbacks.container_loaded = callback_container_loaded;
		callbacks.playlist_added = callback_playlist_added;
		callbacks.playlist_moved = callback_playlist_moved;
		callbacks.playlist_removed = callback_playlist_removed;
	}

	bool PlayListContainer::Load( sp_playlistcontainer* container )
	{
		m_pContainer = container;

		sp_playlistcontainer_callbacks callbacks;
		GetCallbacks( callbacks );

		sp_playlistcontainer_add_callbacks( m_pContainer, &callbacks, this );

		m_isLoading = true;

		return true;
	}

	void PlayListContainer::Unload()
	{
		if (m_pContainer)
		{
			sp_playlistcontainer_callbacks callbacks;
			GetCallbacks( callbacks );

			sp_playlistcontainer_remove_callbacks( m_pContainer, &callbacks, this );

			m_pContainer = NULL;

			int numPlayLists = m_playLists.size();
			for (int i=0; i<numPlayLists; i++)
			{
				delete (m_playLists[i]);
			}
			m_playLists.clear();

			m_isLoading = false;
		}
	}	

	bool PlayListContainer::IsLoading( bool recursive )
	{
		if ( m_isLoading )
		{
			return true;
		}

		if (recursive)
		{
			int numPlayLists = GetNumChildren();
			for (int i=0; i<numPlayLists; i++)
			{
				if (m_playLists[i]->IsLoading( recursive ))
				{
					return true;
				}
			}
		}

		return false;
	}

	void PlayListContainer::AddPlayList( PlayListElement* pPlayList )
	{
		pPlayList->SetParent( this );
		m_playLists.push_back( pPlayList );
	}

	bool PlayListContainer::HasChildren()
	{
		return !m_playLists.empty();
	}

	int PlayListContainer::GetNumChildren()
	{
		return m_playLists.size();
	}

	PlayListElement* PlayListContainer::GetChild( int index )
	{
		return m_playLists[index];
	}

	void PlayListContainer::DumpToTTY( int level )
	{
		//Debug::PrintLine(level, "PlayListContainer" );

		level ++;

		int numPlayLists = GetNumChildren();
		for (int i=0; i<numPlayLists; i++)
		{
			GetChild(i)->DumpToTTY( level );
		}
	}

	std::string PlayListContainer::GetName()
	{
		return "Container";
	}

	PlayListContainer* PlayListContainer::GetPlayListContainer( sp_playlistcontainer* pc, void* userdata )
	{
		PlayListContainer* pContainer = reinterpret_cast<PlayListContainer*>( userdata );
		assert( pContainer->m_pContainer == pc );

		return pContainer;
	}

	void PlayListContainer::callback_playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
	{
		PlayListContainer* pContainer = GetPlayListContainer( pc, userdata );
		pContainer->OnPlaylistAdded( playlist, position );
	}

	void PlayListContainer::callback_playlist_removed(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
	{		
		PlayListContainer* pContainer = GetPlayListContainer( pc, userdata );
		pContainer->OnPlaylistRemoved( playlist, position );
	}

	void PlayListContainer::callback_playlist_moved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata)
	{
		PlayListContainer* pContainer = GetPlayListContainer( pc, userdata );
		pContainer->OnPlaylistMoved( playlist, position, new_position );
	}

	void PlayListContainer::callback_container_loaded(sp_playlistcontainer *pc, void *userdata)
	{
		PlayListContainer* pContainer = GetPlayListContainer( pc, userdata );
		
		pContainer->m_isLoading = false;

		pContainer->OnContainerLoaded();		
	}

	void PlayListContainer::OnPlaylistAdded(sp_playlist *playlist, int position)
	{
		LOG("OnPlaylistAdded [0x%08X]", playlist);
	}

	void PlayListContainer::OnPlaylistRemoved(sp_playlist *playlist, int position)
	{
		LOG("OnPlaylistRemoved [0x%08X]", playlist);
	}

	void PlayListContainer::OnPlaylistMoved(sp_playlist *playlist, int position, int newPosition)
	{
		LOG("OnPlaylistMoved [0x%08X]", playlist);
	}

	void PlayListContainer::OnContainerLoaded()
	{		
		LOG("OnContainerLoaded");	

		int numPlaylists = sp_playlistcontainer_num_playlists( m_pContainer );

		PlayListElement* itContainer = this;
		
		for (int i=0; (i<numPlaylists); i++)
		{
			sp_playlist_type type = sp_playlistcontainer_playlist_type( m_pContainer, i );

			switch ( type )
			{
			case SP_PLAYLIST_TYPE_PLAYLIST:
				{
					sp_playlist* playlist = sp_playlistcontainer_playlist( m_pContainer, i );
					
					PlayList* pPlayList = m_pSession->CreatePlayList();
					pPlayList->Load( playlist );

					itContainer->AddPlayList( pPlayList );					
				}
				break;
			case SP_PLAYLIST_TYPE_START_FOLDER:
				{										
					PlayListFolder* pFolder = m_pSession->CreatePlayListFolder();
					pFolder->Load( m_pContainer, i );

					itContainer->AddPlayList( pFolder );
					itContainer = pFolder;
				}
				break;
			case SP_PLAYLIST_TYPE_END_FOLDER:
					itContainer = itContainer->GetParent();
				break;
			case SP_PLAYLIST_TYPE_PLACEHOLDER:
			default:
				LOG("OTHER???");
				
				// ??
				break;
			}			
			
		}

		assert( itContainer == this );

	}
}
