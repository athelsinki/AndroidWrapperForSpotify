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

// Lib Includes
#include "Spotify/PlayListFolder.h"

// Local Includes
//#include "Debug/Debug.h"

namespace Spotify
{
	PlayListFolder::PlayListFolder( Session* pSession ) : PlayListElement( pSession ), m_pContainer(NULL), m_containerIndex(-1)
	{
	}

	PlayListFolder::~PlayListFolder()
	{
		Unload();
	}

	bool PlayListFolder::IsLoading( bool recursive )
	{
		int numPlayLists = m_playLists.size();

		if (!m_pContainer)
		{
			return false;
		}

		if (recursive)
		{
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

	PlayListElement::eType PlayListFolder::GetType()
	{
		return PLAYLIST_FOLDER;
	}
	
	bool PlayListFolder::Load( sp_playlistcontainer* container, int index )
	{		
		m_pContainer = container;
		m_containerIndex = index;
		
		return true;
	}

	void PlayListFolder::Unload()
	{
		m_pContainer = NULL;
		m_containerIndex = -1;
	}

	void PlayListFolder::AddPlayList( PlayListElement* pPlayList )
	{
		pPlayList->SetParent( this );
		m_playLists.push_back( pPlayList );
	}
	
	std::string PlayListFolder::GetName()
	{
		const int BUFFER_SIZE = 256;
		char buffer[BUFFER_SIZE];
		sp_playlistcontainer_playlist_folder_name( m_pContainer, m_containerIndex, buffer, BUFFER_SIZE );

		std::string folderName = buffer;
		return folderName;
	}

	sp_uint64 PlayListFolder::GetGroupID()
	{
		sp_uint64 groupID = sp_playlistcontainer_playlist_folder_id( m_pContainer, m_containerIndex );
		return groupID;
	}

	bool PlayListFolder::HasChildren()
	{
		return !m_playLists.empty();
	}

	int PlayListFolder::GetNumChildren()
	{
		return m_playLists.size();
	}

	PlayListElement* PlayListFolder::GetChild( int index )
	{
		return m_playLists[index];
	}

	void PlayListFolder::DumpToTTY( int level )
	{
		//Debug::PrintLine( level, "Folder [%s]", GetName().c_str() );

		level ++;

		int numPlayLists = GetNumChildren();

		for (int i=0; i<numPlayLists; i++)
		{
			GetChild(i)->DumpToTTY( level );
		}
	}
}
