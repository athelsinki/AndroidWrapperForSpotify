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
#include <string>
#include <stdio.h>

// libspotify include
#include "libspotify/api.h"


namespace Spotify
{
	// forward declarations
	class Session;
	
	class PlayListElement
	{
	public:
		PlayListElement( Session* pSession );
		virtual ~PlayListElement();

		virtual PlayListElement* GetParent() const;
		virtual void SetParent( PlayListElement* pParent );

		virtual bool HasChildren() = 0;
		virtual int GetNumChildren() = 0;
		virtual PlayListElement* GetChild( int index ) = 0;
		
		virtual bool IsLoading( bool recursive ) = 0;

		virtual std::string GetName() = 0;

		enum eType
		{
			PLAYLIST = 0,
			PLAYLIST_FOLDER,
			PLAYLIST_CONTAINER,
			TRACK
		};

		virtual eType GetType() = 0;

		virtual void AddPlayList( PlayListElement* pPlayList ) {}

		virtual void DumpToTTY( int level = 0 ) = 0;

		void*				GetUserData();
		void				SetUserData( void* pUserData );

		Session* GetSession();

	protected:
		
		PlayListElement*	m_pParent;

		Session*			m_pSession;

	private:
		void*				m_pUserData;
	};

}
