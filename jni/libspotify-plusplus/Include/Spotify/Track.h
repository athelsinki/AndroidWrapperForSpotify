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

// std include
#include <string>
#include <stdio.h>

// libspotify include
#include "libspotify/api.h"

// Local Includes
#include "Spotify/PlayListElement.h"
#include "Spotify/Artist.h"
#include "Spotify/Album.h"

namespace Spotify
{
	// forward declaration
	class Session;

	class Track : public PlayListElement
	{
	public:
		Track( Session* pSession );
		virtual ~Track();

		virtual bool Load(const char* ctrackid);
		virtual bool Load( sp_track* track );

		virtual void Unload();

		virtual bool IsLoading( bool recursive );

		virtual std::string GetName();

		virtual int GetDuration();

		virtual bool HasChildren();
		virtual int GetNumChildren();
		virtual PlayListElement* GetChild( int index );

		virtual eType GetType();

		virtual void DumpToTTY( int level = 0 );

		virtual int GetNumArtists();
		virtual Artist* GetArtist( int index );

		virtual Album* GetAlbum();

	private:
		friend class Session;

		sp_track*	spTrack;
		sp_link*	spLink;
	};
}
