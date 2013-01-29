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

package com.Spotify;

import com.steamrepublic.backstage.app.Backstage;
import com.steamrepublic.backstage.data.SongItem;

import android.util.Log;

public class Session 
{
	//
	//java functions
	//
	public int Initialise(Config config) 
	{		
		m_nativePtr = NativeCreate();
		return Initialise(m_nativePtr, config);
	}
	
	public int getNativePtr()
	{
		return m_nativePtr;
	}

	public void Shutdown() 
	{
		Shutdown(m_nativePtr);

		NativeDestroy(m_nativePtr);

		m_nativePtr = 0;
	}

	public void Update() 
	{
		Log.d("Backstage", "***** Java:Update: ");
		Update(m_nativePtr);
	}

	public void Login(String username, String password) 
	{
		Login(m_nativePtr, username, password);
	}

	public void Logout() 
	{
		Logout(m_nativePtr);
	}

	public boolean IsLoggedIn() 
	{
		return IsLoggedIn(m_nativePtr);
	}

	final int SP_CONNECTION_STATE_LOGGED_OUT = 0; // /< User not yet logged in
	final int SP_CONNECTION_STATE_LOGGED_IN = 1; // /< Logged in against a
													// Spotify access point
	final int SP_CONNECTION_STATE_DISCONNECTED = 2; // /< Was logged in, but has
													// now been disconnected
	final int SP_CONNECTION_STATE_UNDEFINED = 3; // /< The connection state is
													// undefined

	public int GetConnectionState() 
	{
		return GetConnectionState(m_nativePtr);
	}

	public int Load(String trackid) 
	{
		return Load(m_nativePtr, trackid);
	}

	public void Unload() 
	{
		Unload(m_nativePtr);
	}

	public Track GetCurrentTrack() 
	{
		return GetCurrentTrack(m_nativePtr);
	}

	public void Seek(int offset) 
	{
		Seek(m_nativePtr, offset);
	}

	public void Play() 
	{		
		Play(m_nativePtr);
	}

	public void Stop() 
	{
		Stop(m_nativePtr);
	}

	public int PreFetch(Track pTrack) 
	{
		return PreFetch(m_nativePtr, pTrack);
	}

	public PlayListContainer GetPlayListContainer() 
	{
		return GetPlayListContainer(m_nativePtr);
	}

	final int SP_BITRATE_160k = 0;
	final int SP_BITRATE_320k = 1;

	public void SetPreferredBitrate(int bitrate) 
	{
		SetPreferredBitrate(m_nativePtr, bitrate);
	}

	//
	//call back functions
	//	
	public void OnLoggedIn(int error) 
	{
		Log.d("Backstage", "---** Java:OnLoggedIn: ");
		Backstage.musicManager.setSpotifyLoggedIn(Backstage.musicManager.spotifySession.IsLoggedIn());
		Log.d("Backstage", "---** Java:IsLoggedIn: " + Backstage.musicManager.isSpotifyLoggedIn());
		//System.out.printf("JAVA - OnLoggedIn - error[%d]\n", error);
	}

	public void OnLoggedOut() 
	{
		Log.d("Backstage", "---** Java:OnLoggedOut: ");
		//System.out.printf("JAVA - OnLoggedOut\n");
	}

	public void OnMetadataUpdated() 
	{
		Log.d("Backstage", "---** Java:OnMetadataUpdated: ");

		//only when load returns 17
		if (Backstage.musicManager.isLoadCalled && !Backstage.musicManager.isLoaded && !Backstage.musicManager.isPlayCalled)
		{
			if(IsLoaded(m_nativePtr, true))
			{
				Log.d("Backstage", "---** Java:OnMetadataUpdated:track loaded ");
				
				Backstage.musicManager.spotifySession.Play();
			
				Backstage.musicManager.isLoaded = true;
				Backstage.musicManager.isPlayCalled = true;
			}
		}
		
		Log.d("Backstage", "---** Java:OnMetadataUpdated:end ");
	}

	public void OnConnectionError(int error) 
	{
		Log.d("Backstage", "---** Java:OnConnectionError: ");
		//System.out.printf(" JAVA - OnConnectionError [%d]\n", error);
	}

	public void OnMessageToUser(String message) 
	{
		Log.d("Backstage", "---** Java:OnMessageToUser: " + message);
		//System.out.printf("JAVA - OnMessageToUser [%s]", message);
	}

	public void OnNotifyMainThread() 
	{
		Log.d("Backstage", "---** Java:OnNotifyMainThread: ");
		//System.out.println("JAVA - OnNotifyMainThread");
	}

	final int SP_SAMPLETYPE_INT16_NATIVE_ENDIAN = 0; // /< 16-bit signed integer
														// samples

	//not called by JNI
	public int OnMusicDelivery(AudioFormat format, byte[] frames, int numFrames) 
	{
		Log.d("Backstage", "---** Java:OnMusicDelivery:m_sampleType:" + format.m_sampleType + ",m_sampleRate:" + format.m_sampleRate + ",m_channels:" + format.m_channels + ",size:" + frames.length + ",numFrames" + numFrames);

		return numFrames;
	}

	public void OnPlayTokenLost() 
	{
		Log.d("Backstage", "---** Java:OnPlayTokenLost: ");
		//System.out.printf("Java - OnPlayTokenLost\n");
	}

	public void OnLogMessage(String message) 
	{
		Log.d("Backstage", "---** Java:OnLogMessage: ");
		//System.out.printf("Java - OnLogMessage [%s]\n", message);
	}

	public void OnEndOfTrack() 
	{
		Log.d("Backstage", "---** Java:OnEndOfTrack: ");
		
		if(Backstage.musicManager.playBtnType == Backstage.musicManager.playBtnType.ALBUM_COVER)
		{
			if(Backstage.musicManager.currentSongIndex + 1 < Backstage.musicManager.currentAlbum.getTracksLoaded())
			{
				SongItem song = Backstage.musicManager.currentAlbum.getTrack(Backstage.musicManager.currentSongIndex + 1);
				Backstage.musicManager.album_song_play(null, song);		//null, the view will be always different. 
				
				//update the album or track info
				Backstage.musicManager.currentSongIndex += 1;
				Backstage.musicManager.currentTrackID = song.getSpotifyTrackID();
				Backstage.musicManager.currentTrackName = song.getName();
			}	
		}
		else
		{
			//should change the play button icon here
		}

		
		Log.d("Backstage", "---** Java:OnEndOfTrack:end ");
	}

	public void OnStreamingError(int error) 
	{
		Log.d("Backstage", "---** Java:OnStreamingError: ");
	}

	public void OnUserinfoUpdated() 
	{
		Log.d("Backstage", "---** Java:OnUserinfoUpdated: ");

	}

	public void OnStartPlayback() 
	{
		Log.d("Backstage", "---** Java:OnStartPlayback: ");
	}

	public void OnStopPlayback() 
	{
		Log.d("Backstage", "---** Java:OnStopPlayback: ");
	}

	public void OnGetAudioBufferStats(AudioBufferStats stats) 
	{
		Log.d("Backstage", "---** Java:OnGetAudioBufferStats: ");

	}

	
	//
	//native functions
	//
	private native int NativeCreate();

	private native void NativeDestroy(int nativePtr);

	private native int Initialise(int nativePtr, Config config);

	private native void Shutdown(int nativePtr);

	private native void Update(int nativePtr);

	private native void Login(int nativePtr, String username, String password);

	private native void Logout(int nativePtr);

	private native boolean IsLoggedIn(int nativePtr);
	
	private native boolean IsLoaded(int nativePtr, boolean recursive);

	private native int GetConnectionState(int nativePtr);

	private native int Load(int nativePtr,String trackid);

	private native void Unload(int nativePtr);

	private native Track GetCurrentTrack(int nativePtr);

	private native void Seek(int nativePtr, int offset);

	private native void Play(int nativePtr);

	private native void Stop(int nativePtr);

	private native int PreFetch(int nativePtr, Track pTrack);

	private native PlayListContainer GetPlayListContainer(int nativePtr);

	private native void SetPreferredBitrate(int nativePtr, int bitrate);

	private int m_nativePtr = 0;
}
