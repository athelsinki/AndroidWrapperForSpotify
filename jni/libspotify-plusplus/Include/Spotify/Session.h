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

// C-libs includes
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

// Local Includes
#include "libspotify/api.h"

namespace Spotify
{
	class Album;
	class Artist;
	class Image;
	class PlayList;
	class PlayListContainer;
	class PlayListElement;
	class PlayListFolder;
	class Track;

	namespace Core
	{
		// forward declaration
		class Mutex;
	}

	class Session
	{
	public:
		
		struct Config
		{
			Config();

			const uint8_t*	m_appKey;
			size_t			m_appKeySize;
			const char*		m_cacheLocation;
			const char*		m_settingsLocation;
			const char*		m_userAgent;
			bool			m_compressPlaylists;
			bool			m_dontSaveMetadataForPlaylists;
			bool			m_initiallyUnloadPlaylists;

		};

		Session();
		virtual ~Session();

		virtual sp_error	Initialise( Config& config );
		virtual void		Shutdown();

		virtual void		Update();

		virtual void		Login( const char* username, const char* password );
		virtual void		Logout();
		
		virtual bool		IsLoggedIn();

		virtual sp_connectionstate GetConnectionState();

		virtual sp_error	Load( Track* pTrack );
		virtual void		Unload( Track* pTrack );
		//virtual Track*		GetCurrentTrack();
		virtual void		Seek( int offset );
		virtual void		Play(Track* pTrack);
		virtual void		Stop();
		
		virtual sp_error	PreFetch( Track* pTrack );

		virtual PlayListContainer*	GetPlayListContainer();

		virtual void		SetPreferredBitrate( sp_bitrate bitrate );

		// factory functions
		virtual PlayList*			CreatePlayList();
		virtual PlayListContainer*	CreatePlayListContainer();
		virtual PlayListFolder*		CreatePlayListFolder();
		virtual Track*				CreateTrack();
		virtual Artist*				CreateArtist();
		virtual Album*				CreateAlbum();
		virtual Image*				CreateImage();

	protected:
		// C++ Style member callbacks
		virtual void OnLoggedIn( sp_error error );
		virtual void OnLoggedOut();
		virtual void OnMetadataUpdated();
		virtual void OnConnectionError( sp_error error );
		virtual void OnMessageToUser( const char* message );
		virtual void OnNotifyMainThread();
		virtual int  OnMusicDelivery( const sp_audioformat* format, const void* frames, int num_frames );
		virtual void OnPlayTokenLost();
		virtual void OnLogMessage( const char* data );
		virtual void OnEndOfTrack();
		virtual void OnStreamingError( sp_error error );
		virtual void OnUserinfoUpdated();
		virtual void OnStartPlayback();
		virtual void OnStopPlayback();
		virtual void OnGetAudioBufferStats( sp_audio_buffer_stats* stats );

	private:

		friend class Image;

		// C Style Static callbacks
		static void SP_CALLCONV callback_logged_in(sp_session *session, sp_error error);
		static void SP_CALLCONV callback_logged_out(sp_session *session);
		static void SP_CALLCONV callback_metadata_updated(sp_session *session);
		static void SP_CALLCONV callback_connection_error(sp_session *session, sp_error error);
		static void SP_CALLCONV callback_message_to_user(sp_session *session, const char *message);
		static void SP_CALLCONV callback_notify_main_thread(sp_session *session);
		static int  SP_CALLCONV callback_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
		static void SP_CALLCONV callback_play_token_lost(sp_session *session);
		static void SP_CALLCONV callback_log_message(sp_session *session, const char *data);
		static void SP_CALLCONV callback_end_of_track(sp_session *session);
		static void SP_CALLCONV callback_streaming_error(sp_session *session, sp_error error);
		static void SP_CALLCONV callback_userinfo_updated(sp_session *session);
		static void SP_CALLCONV callback_start_playback(sp_session *session);
		static void SP_CALLCONV callback_stop_playback(sp_session *session);
		static void SP_CALLCONV callback_get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats);
		static void SP_CALLCONV callback_offline_status_updated(sp_session *session);
		static void SP_CALLCONV callback_offline_error(sp_session *session, sp_error error);
		static void SP_CALLCONV callback_credentials_blob_updated(sp_session *session, const char *blob);
		static void SP_CALLCONV callback_connectionstate_updated(sp_session *session);
		static void SP_CALLCONV callback_scrobble_error(sp_session *session, sp_error error);
		static void SP_CALLCONV callback_private_session_mode_changed(sp_session *session, bool is_private);

		
		sp_session*		m_pSession;
		
		volatile bool	m_isProcessEventsRequired;
		pthread_cond_t  g_notify_cond;

		volatile bool	m_hasLoggedOut;


		//Track*			m_pTrack;		// currently playing track
		Core::Mutex*	m_mutex;
	};

}

