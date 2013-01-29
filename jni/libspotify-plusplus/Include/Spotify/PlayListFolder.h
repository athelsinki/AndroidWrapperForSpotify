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

// std includes
#include <vector>
#include <string>
#include <stdio.h>

// libspotify Includes
#include "libspotify/api.h"

// Local Includes
#include "Spotify/PlayListElement.h"


namespace Spotify
{
	// forward declaration
	class Session;

	class PlayListFolder : public PlayListElement
	{
	public:
		
		PlayListFolder( Session* pSession );
		virtual ~PlayListFolder();

		virtual bool Load( sp_playlistcontainer* container, int index ); 
		virtual void Unload();

		virtual void AddPlayList( PlayListElement* pPlayList );

		virtual bool IsLoading( bool recursive );

		virtual eType GetType();

		virtual void DumpToTTY( int level = 0 );

		virtual std::string GetName();

		virtual sp_uint64 GetGroupID();

		virtual bool HasChildren();
		virtual int	GetNumChildren();
		virtual PlayListElement* GetChild( int index );

	private:
		
		typedef std::vector<PlayListElement*>	PlayListStore;
		PlayListStore					m_playLists;

		sp_playlistcontainer*			m_pContainer;
		int								m_containerIndex;
	};
}
