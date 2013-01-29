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

#include "JTrack.h"
#include "JSession.h"
#include "JArtist.h"
#include "JAlbum.h"

#include "JUtils.h"

// JNI
#include "Spotify/Spotify_Track.h"

/*
JNIEXPORT jboolean JNICALL Java_com_Spotify_Track_Load
  (JNIEnv *env, jobject object, jstring trackid)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, object );
	Spotify::Track* pTrack = static_cast<Spotify::Track*>( pElement );

	jboolean isCopy;
	const char* szTrackid = env->GetStringUTFChars(trackid, &isCopy );

	jboolean isLoad;
	isLoad = pTrack->Load(szTrackid);

	env->ReleaseStringUTFChars( trackid, szTrackid );

	return isLoad;
}*/

JNIEXPORT jint JNICALL Java_com_Spotify_Track_GetDuration
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, object );
	Spotify::Track* pTrack = static_cast<Spotify::Track*>( pElement );
	
	return pTrack->GetDuration();
}

JNIEXPORT jint JNICALL Java_com_Spotify_Track_GetNumArtists
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, object );
	Spotify::Track* pTrack = static_cast<Spotify::Track*>( pElement );

	return pTrack->GetNumArtists();
}

JNIEXPORT jobject JNICALL Java_com_Spotify_Track_GetArtist
  (JNIEnv *env, jobject object, jint index)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, object );
	Spotify::Track* pTrack = static_cast<Spotify::Track*>( pElement );

	Spotify::JArtist* pArtist = static_cast<Spotify::JArtist*>( pTrack->GetArtist( index ) );

	jclass cls = env->FindClass( "com/Spotify/Artist" );
	jmethodID cid = env->GetMethodID( cls, "<init>", "(I)V");
		
	jobject javaObject = env->NewObject( cls, cid, PointerToNativePtr(pArtist) );

	return javaObject;
}

JNIEXPORT jobject JNICALL Java_com_Spotify_Track_GetAlbum
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, object );
	Spotify::Track* pTrack = static_cast<Spotify::Track*>( pElement );

	Spotify::JAlbum* pAlbum = static_cast<Spotify::JAlbum*>( pTrack->GetAlbum() );

	jclass cls = env->FindClass( "com/Spotify/Album" );
	jmethodID cid = env->GetMethodID( cls, "<init>", "(I)V");
		
	jobject javaObject = env->NewObject( cls, cid, PointerToNativePtr(pAlbum) );

	return javaObject;
}

namespace Spotify
{

	JTrack::JTrack( JSession* pSession ) : Track( pSession )
	{
		InitialiseJavaObject( pSession->GetEnv(), this, "com/Spotify/Track" );
	}

}
