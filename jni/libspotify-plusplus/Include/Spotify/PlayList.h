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

// libspotify include
#include "libspotify/api.h"

// local includes
#include "Spotify/PlayListElement.h"

namespace Spotify
{
	// forward declarations
	class Session;
	class Track;
	
	class PlayList : public PlayListElement
	{
	public:
		PlayList( Session* pSession );
		virtual ~PlayList();

		virtual eType GetType();

		virtual bool Load( sp_playlist* playlist );	
		virtual void Unload();
		
		virtual bool IsLoading( bool recursive );		

		virtual int GetNumTracks();

		virtual Track* GetTrack( int index );

		virtual std::string GetName();

		virtual bool HasChildren();
		virtual int GetNumChildren();
		virtual PlayListElement* GetChild( int index );

		virtual void DumpToTTY( int level = 0 );
		
	protected:
		virtual void OnTracksAdded(sp_track * const *tracks, int numTracks, int position);
		virtual void OnTracksRemoved(const int *tracks, int numTracks);
		virtual void OnTracksMoved(const int *tracks, int numTracks, int newPosition);
		virtual void OnPlaylistRenamed();
		virtual void OnPlaylistStateChanged();
		virtual void OnPlaylistUpdateInProgress(bool done);
		virtual void OnPlaylistMetadataUpdated();
		virtual void OnTrackCreatedChanged(int position, sp_user *user, int when);
		virtual void OnTrackSeenChanged(int position, bool seen);
		virtual void OnDescriptionChanged(const char *desc);
		virtual void OnImageChanged(const byte *image);

		virtual void LoadTracks();

	private:
		friend class Session;

		static void SP_CALLCONV callback_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position, void *userdata);
		static void SP_CALLCONV callback_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata);
		static void SP_CALLCONV callback_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata);
		static void SP_CALLCONV callback_playlist_renamed(sp_playlist *pl, void *userdata);
		static void SP_CALLCONV callback_playlist_state_changed(sp_playlist *pl, void *userdata);
		static void SP_CALLCONV callback_playlist_update_in_progress(sp_playlist *pl, bool done, void *userdata);
		static void SP_CALLCONV callback_playlist_metadata_updated(sp_playlist *pl, void *userdata);
		static void SP_CALLCONV callback_track_created_changed(sp_playlist *pl, int position, sp_user *user, int when, void *userdata);
		static void SP_CALLCONV callback_track_seen_changed(sp_playlist *pl, int position, bool seen, void *userdata);
		static void SP_CALLCONV callback_description_changed(sp_playlist *pl, const char *desc, void *userdata);
		static void SP_CALLCONV callback_image_changed(sp_playlist *pl, const byte *image, void *userdata);

		static void GetCallbacks( sp_playlist_callbacks& callbacks );
		static PlayList* GetPlayListFromUserData( sp_playlist* pl, void* userdata );
						
		sp_playlist* m_pPlayList;
		
		bool m_isLoading;

		typedef std::vector<Track*> TrackStore;
		TrackStore		m_tracks;
	};
}
