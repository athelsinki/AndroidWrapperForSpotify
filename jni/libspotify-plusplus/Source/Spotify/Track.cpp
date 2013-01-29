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

// lib includes
#include "Spotify/Track.h"
#include "Spotify/Session.h"

// Local includes
//#include "Debug/Debug.h"
#define LOG_TAG "MBS"
#include <android/log.h>

namespace Spotify
{

	Track::Track( Session* pSession ) : PlayListElement( pSession ), spTrack(NULL), spLink(NULL)
	{
	}

	Track::~Track()
	{
		Unload();
	}
	
	bool Track::Load(const char* ctrackid)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Track::Load with trackid");

		spLink = sp_link_create_from_string(ctrackid);
		spTrack = sp_link_as_track(spLink);
		sp_track_add_ref( spTrack );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Track::Load with trackid:end");

		return true;
	}

	bool Track::Load( sp_track* track )
	{
		spTrack = track;
		sp_track_add_ref( spTrack );
		return true;
	}

	void Track::Unload()
	{
		sp_track_release( spTrack );
		spTrack = NULL;
		spLink = NULL;
	}

	bool Track::IsLoading( bool recursive )
	{
		if (!sp_track_is_loaded(spTrack))
			return true;

		return false;
	}			

	std::string Track::GetName()
	{
		const char* name = sp_track_name( spTrack );
		return name;
	}

	int Track::GetDuration()
	{
		int duration = sp_track_duration( spTrack );
		return duration;
	}

	PlayListElement::eType Track::GetType()
	{
		return TRACK;
	}

	bool Track::HasChildren()
	{
		return false;
	}

	int Track::GetNumChildren()
	{
		return 0;
	}

	PlayListElement* Track::GetChild( int index )
	{
		return NULL;
	}

	void Track::DumpToTTY( int level )
	{
		int seconds = GetDuration() / 1000;
		int mins = seconds / 60;
		seconds %= 60;
		//Debug::PrintLine( level, "Track [%s] [%d]mins [%d]secs", GetName().c_str(), mins, seconds);
	}

	int Track::GetNumArtists()
	{
		return sp_track_num_artists( spTrack );
	}

	Artist* Track::GetArtist( int index )
	{
		Artist* pArtist = m_pSession->CreateArtist();
		pArtist->Load( sp_track_artist( spTrack, index ) );

		return pArtist;
	}

	Album* Track::GetAlbum()
	{
		Album* pAlbum = m_pSession->CreateAlbum();
		pAlbum->Load( sp_track_album( spTrack ) );

		return pAlbum;
	}
}
