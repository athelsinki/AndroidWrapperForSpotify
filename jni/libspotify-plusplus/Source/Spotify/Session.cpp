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

// Includes
#include "Spotify/Session.h"

#include "Spotify/Album.h"
#include "Spotify/Artist.h"
#include "Spotify/Image.h"
#include "Spotify/PlayList.h"
#include "Spotify/PlayListContainer.h"
#include "Spotify/PlayListElement.h"
#include "Spotify/PlayListFolder.h"
#include "Spotify/Track.h"
#include "Spotify/Core/Mutex.h"
#include "Spotify/sound_driver.h"

// debugging
//#include "Debug/Debug.h"
#define LOG( msg, ... )	//Debug::PrintLine( msg, __VA_ARGS__ );

#include <android/log.h>
#define LOG_TAG "MBS"

namespace Spotify
{
	Session::Config::Config()
	{
		m_appKey = NULL;
		m_appKeySize = 0;
		m_cacheLocation = "";
		m_settingsLocation = "";
		m_userAgent = "";
		m_compressPlaylists = true;
		m_dontSaveMetadataForPlaylists = false;
		m_initiallyUnloadPlaylists = false;
	}

	Session::Session() : m_pSession(NULL), m_isProcessEventsRequired(false), m_hasLoggedOut(NULL)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "---Session:Session()");

		m_mutex = new Core::Mutex();
		pthread_cond_init(&g_notify_cond, NULL);

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "---Session:Session():end");
	}

	Session::~Session()
	{		
		Shutdown();
	}

	sp_error Session::Initialise( Config& config )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise");

		sp_session_config spConfig;
		memset( &spConfig, 0, sizeof(spConfig) );
		
		spConfig.api_version = SPOTIFY_API_VERSION;
		
		// app specified configuration
		spConfig.application_key = config.m_appKey;
		spConfig.application_key_size = config.m_appKeySize;
		spConfig.cache_location = config.m_cacheLocation;
		spConfig.settings_location = config.m_settingsLocation;
		spConfig.user_agent = config.m_userAgent;
		
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:key:%s", spConfig.application_key );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:key_size:%d", spConfig.application_key_size );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:cache_location:%s", spConfig.cache_location );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:settings_location:%s", spConfig.settings_location );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:user_agent:%s", spConfig.user_agent );

		// locally specified configuration
		sp_session_callbacks callbacks;
		memset( &callbacks, 0, sizeof(callbacks) );
		

		callbacks.logged_in = callback_logged_in;
		callbacks.logged_out = callback_logged_out;
		callbacks.metadata_updated = callback_metadata_updated;
		callbacks.connection_error = callback_connection_error;
		callbacks.message_to_user = callback_message_to_user;
		callbacks.notify_main_thread = callback_notify_main_thread;
		callbacks.music_delivery = callback_music_delivery;
		callbacks.play_token_lost = callback_play_token_lost;
		callbacks.log_message = callback_log_message;
		callbacks.end_of_track = callback_end_of_track;
		callbacks.streaming_error = callback_streaming_error;
		callbacks.userinfo_updated = callback_userinfo_updated;
		callbacks.start_playback = callback_start_playback;
		callbacks.stop_playback = callback_stop_playback;
		//callbacks.get_audio_buffer_stats = callback_get_audio_buffer_stats;
		callbacks.offline_status_updated = callback_offline_status_updated;
		callbacks.offline_error = callback_offline_error;
		callbacks.credentials_blob_updated = callback_credentials_blob_updated;
		callbacks.connectionstate_updated = callback_connectionstate_updated;
		callbacks.scrobble_error = callback_scrobble_error;
		callbacks.private_session_mode_changed = callback_private_session_mode_changed;
		
		spConfig.callbacks = &callbacks;
		spConfig.userdata = this;			

		spConfig.compress_playlists = config.m_compressPlaylists;
		spConfig.dont_save_metadata_for_playlists = config.m_dontSaveMetadataForPlaylists;
		spConfig.initially_unload_playlists = config.m_initiallyUnloadPlaylists;

		sp_error error = sp_session_create( &spConfig, &m_pSession );

		//Initialize the native audio player
		init_audio_player();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Initialise:end:%d", error);

		return error;
	}

	void Session::Shutdown()
	{
		if (m_pSession)
		{			
			// release the session
			sp_session_release( m_pSession );
			m_pSession = NULL;
			
			//destroy the native audio player
			destroy_audio_player();
		}		
	}

	void Session::Update()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():m_pSession - %x", m_pSession);
		if (m_pSession)
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():in");
			int nextTimeout = 0;

			pthread_mutex_lock(m_mutex->theMutex);
			for(;;)
			{
				__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():in for");
				if (nextTimeout == 0)
				{
					__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():nextTimeout is 0");
					while(!m_isProcessEventsRequired)
					{
						__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():pthread_cond_wait");
						pthread_cond_wait(&g_notify_cond, m_mutex->theMutex);
					}
				}
				else
				{
					struct timespec ts;
					memset( &ts, 0, sizeof(ts) );

					#if _POSIX_TIMERS > 0
								clock_gettime(CLOCK_REALTIME, &ts);
					#else
								struct timeval tv;
								gettimeofday(&tv, NULL);
								TIMEVAL_TO_TIMESPEC(&tv, &ts);
					#endif

					ts.tv_sec += nextTimeout / 1000;
					ts.tv_nsec += (nextTimeout % 1000) * 1000000;

					__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():pthread_cond_timedwait:%d,%f", ts.tv_sec,ts.tv_nsec);
					pthread_cond_timedwait(&g_notify_cond, m_mutex->theMutex, &ts);
				}

				m_isProcessEventsRequired = false;
				pthread_mutex_unlock(m_mutex->theMutex);

				do
				{
					__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():sp_session_process_events");
					sp_session_process_events( m_pSession, &nextTimeout );
					__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():sp_session_process_events:end:nextTimeout-%d", nextTimeout);

				}while(nextTimeout == 0);

				pthread_mutex_lock(m_mutex->theMutex);
				__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update():get mutex after sp_session_process_events ");
			}
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** Session::Update:end");
	}

	void Session::Login( const char* username, const char* password )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:login:%s,%s", username, password);

		m_hasLoggedOut = false;

		sp_error error;
		error = sp_session_login( m_pSession, username, password, false, NULL);

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:login:end:%d", error);
	}

	void Session::Logout()
	{		
		sp_session_logout( m_pSession );
	}

	bool Session::IsLoggedIn()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:IsLoggedIn");
		sp_connectionstate connectionState = GetConnectionState();

		bool isloggedIn = (NULL != m_pSession) && !m_hasLoggedOut && (connectionState == SP_CONNECTION_STATE_LOGGED_IN);

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:IsLoggedIn:%d,%d,%d", m_pSession, m_hasLoggedOut, connectionState);
		return isloggedIn;
	}

	sp_connectionstate Session::GetConnectionState()
	{
		if (m_pSession)
		{
			return sp_session_connectionstate(m_pSession);
		}
		else
		{
			return SP_CONNECTION_STATE_LOGGED_OUT;
		}
	}

	sp_error Session::Load( Track* pTrack )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Load:%s,%s",pTrack,pTrack->spTrack);

		if (pTrack)
		{
			sp_error error = sp_session_player_load( m_pSession, pTrack->spTrack );

			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Load:end:sperror:%d", error);

			return error;
		}


		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Load: Track NULL");
		return SP_ERROR_OK;
	}

	void Session::Unload( Track* pTrack )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Load:%s,%s",pTrack,pTrack->spTrack);
		if (pTrack)
		{
			sp_session_player_unload( m_pSession );
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Unload");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:Unload:end");
	}

	/*
	Track* Session::GetCurrentTrack()
	{
		return m_pTrack;
	}*/

	void Session::Seek( int offset )
	{
		sp_session_player_seek( m_pSession, offset );		
	}

	void Session::Play(Track* pTrack)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:Play()");
		sp_session_player_play( m_pSession, true );

		sp_track *track = sp_track_get_playable(m_pSession, pTrack->spTrack);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:Play():end:%s",track);
	}

	void Session::Stop()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:Stop()");

		sp_session_player_play( m_pSession, false );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:Stop():end");
	}
		
	sp_error Session::PreFetch( Track* pTrack )
	{
		sp_error error = sp_session_player_prefetch( m_pSession, pTrack->spTrack );
		return error;
	}

	PlayListContainer* Session::GetPlayListContainer()
	{
		sp_playlistcontainer* container = sp_session_playlistcontainer( m_pSession );
		
		if (NULL == container)
		{
			return NULL;
		}

		PlayListContainer* pContainer = CreatePlayListContainer();
		bool isLoading = pContainer->Load( container );

		if (!isLoading)
		{
			delete pContainer;
			pContainer = NULL;
		}

		return pContainer;
	}

	void Session::SetPreferredBitrate( sp_bitrate bitrate )
	{
		sp_session_preferred_bitrate( m_pSession, bitrate );
	}

	PlayList* Session::CreatePlayList()
	{
		return new PlayList( this );
	}

	PlayListContainer* Session::CreatePlayListContainer()
	{
		return new PlayListContainer( this );
	}

	PlayListFolder* Session::CreatePlayListFolder()
	{
		return new PlayListFolder( this );
	}

	Track* Session::CreateTrack()
	{
		return new Track( this );
	}

	Artist* Session::CreateArtist()
	{
		return new Artist( this );
	}

	Album* Session::CreateAlbum()
	{
		return new Album( this );
	}

	Image* Session::CreateImage()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:CreateImage");
		return new Image( this );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:CreateImage:end");
	}
		
	static Session* GetSessionFromUserdata( sp_session* session )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:GetSessionFromUserdata");
		Session* pSession = reinterpret_cast<Session*>(sp_session_userdata(session));
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:GetSessionFromUserdata:end");
		return pSession;
	}

	void SP_CALLCONV Session::callback_logged_in(sp_session *session, sp_error error)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_logged_in");

		Session* pSession = GetSessionFromUserdata( session );		
		pSession->OnLoggedIn( error );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_logged_in:end");
	}

	void SP_CALLCONV Session::callback_logged_out(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_logged_out");
		Session* pSession = GetSessionFromUserdata( session );
		
		pSession->m_hasLoggedOut = true;
		
		pSession->OnLoggedOut();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_logged_out:end");
	}

	void SP_CALLCONV Session::callback_metadata_updated(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_metadata_updated");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnMetadataUpdated();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_metadata_updated:end");
	}

	void SP_CALLCONV Session::callback_connection_error(sp_session *session, sp_error error)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_connection_error");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnConnectionError( error );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_connection_error:end");

	}

	void SP_CALLCONV Session::callback_message_to_user(sp_session *session, const char *message)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_message_to_user");
		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnMessageToUser( message );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_message_to_user:end");
	}

	void SP_CALLCONV Session::callback_notify_main_thread(sp_session *session)
	{		
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_notify_main_thread");
		Session* pSession = GetSessionFromUserdata( session );

		pSession->OnNotifyMainThread();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_notify_main_thread:end");
	}

	int  SP_CALLCONV Session::callback_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_music_delivery");

		Session* pSession = GetSessionFromUserdata( session );
		return pSession->OnMusicDelivery( format, frames, num_frames );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_music_delivery:end");
	}

	void SP_CALLCONV Session::callback_play_token_lost(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_play_token_lost");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnPlayTokenLost();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_play_token_lost:end");
	}

	void SP_CALLCONV Session::callback_log_message(sp_session *session, const char *data)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_log_message");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnLogMessage( data );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_log_message:end");
	}

	void SP_CALLCONV Session::callback_end_of_track(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_end_of_track");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnEndOfTrack();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_end_of_track:end");
	}

	void SP_CALLCONV Session::callback_streaming_error(sp_session *session, sp_error error)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_streaming_error");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnStreamingError( error );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_streaming_error:end");
	}

	void SP_CALLCONV Session::callback_userinfo_updated(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_userinfo_updated");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnUserinfoUpdated();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_userinfo_updated:end");
	}

	void SP_CALLCONV Session::callback_start_playback(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_start_playback");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnStartPlayback();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_start_playback:end");
	}

	void SP_CALLCONV Session::callback_stop_playback(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_stop_playback");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnStopPlayback();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_stop_playback:end");
	}

	void SP_CALLCONV Session::callback_get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_get_audio_buffer_stats");

		Session* pSession = GetSessionFromUserdata( session );
		pSession->OnGetAudioBufferStats( stats );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_get_audio_buffer_stats:end");
	}

	void SP_CALLCONV Session::callback_offline_status_updated(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_offline_status_updated");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_offline_status_updated:end");
	}

	void SP_CALLCONV Session::callback_offline_error(sp_session *session, sp_error error)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_offline_error");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:callback_offline_error:%d ", error);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_offline_error:end");
	}

	void SP_CALLCONV Session::callback_credentials_blob_updated(sp_session *session, const char *blob)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_credentials_blob_updated");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_credentials_blob_updated:end:credential:%s", blob);
	}

	void SP_CALLCONV Session::callback_connectionstate_updated(sp_session *session)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_connectionstate_updated");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:callback_connectionstate_updated:%d ", sp_session_connectionstate(session));

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_connectionstate_updated:end");
	}

	void SP_CALLCONV Session::callback_scrobble_error(sp_session *session, sp_error error)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_scrobble_error");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:callback_scrobble_error:%d ", error);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_scrobble_error:end");
	}

	void SP_CALLCONV Session::callback_private_session_mode_changed(sp_session *session, bool is_private)
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_private_session_mode_changed");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:callback_private_session_mode_changed:%d ", is_private);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Session:callback_private_session_mode_changed:end");
	}


	void Session::OnLoggedIn( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLoggedIn");
		LOG("Session::OnLoggedIn");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLoggedIn:end");
	}

	void Session::OnLoggedOut()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLoggedOut");
		LOG("Session::OnLoggedOut");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLoggedOut:end");
	}

	void Session::OnMetadataUpdated()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnMetadataUpdated");
		LOG("Session::OnMetadataUpdated");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnMetadataUpdated:end");
	}

	void Session::OnConnectionError( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnConnectionError");
		LOG("Session::OnConnectionError");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnConnectionError:end");
	}

	void Session::OnMessageToUser( const char* message )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnMessageToUser");
		LOG("Session::OnMessageToUser");
		LOG( message );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnMessageToUser:end");
	}

	void Session::OnNotifyMainThread()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnNotifyMainThread");
		LOG("Session::OnNotifyMainThread");

		pthread_mutex_lock(m_mutex->theMutex);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnNotifyMainThread: get Mutex");
		m_isProcessEventsRequired = true;
		pthread_cond_signal(&g_notify_cond);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnNotifyMainThread: pthread_cond_signal");
		pthread_mutex_unlock(m_mutex->theMutex);

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnNotifyMainThread:end");
	}

	int  Session::OnMusicDelivery( const sp_audioformat* format, const void* frames, int num_frames )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnMusicDelivery");

		 return music_delivery(m_pSession, format, frames, num_frames);


	}

	void Session::OnPlayTokenLost()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnPlayTokenLost");
		LOG("Session::OnPlayTokenLost");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnPlayTokenLost:end");
	}

	void Session::OnLogMessage( const char* data )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLogMessage");
		LOG("Session::OnLogMessage");
		LOG( data );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLogMessage, data: %s", data);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLogMessage:end");
	}

	void Session::OnEndOfTrack()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnEndOfTrack");
		LOG("Session::OnEndOfTrack");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnEndOfTrack:end");
	}

	void Session::OnStreamingError( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStreamingError");
		LOG("Session::OnStreamingError");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStreamingError:end");
	}

	void Session::OnUserinfoUpdated()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnUserinfoUpdated");
		LOG("Session::OnUserinfoUpdated");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnUserinfoUpdated:end");
	}

	void Session::OnStartPlayback()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStartPlayback");
		LOG("Session::OnStartPlayback");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStartPlayback:end");
	}

	void Session::OnStopPlayback()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStopPlayback");
		LOG("Session::OnStopPlayback");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnStopPlayback:end");
	}

	void Session::OnGetAudioBufferStats( sp_audio_buffer_stats* stats )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnGetAudioBufferStats");
		LOG("Session::OnGetAudioBufferStats");
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnLogMessage, stats: %d,%d", stats->samples, stats->stutter);
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Session:OnGetAudioBufferStats:end");
	}
	
}
