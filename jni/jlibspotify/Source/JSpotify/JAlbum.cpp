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

#include "JAlbum.h"
#include "JImage.h"
#include "JUtils.h"
#include "JSession.h"

#include "Spotify/Spotify_Album.h"

Spotify::JAlbum* GetAlbumNativePtr( JNIEnv* env, jobject object )
{
	jclass cls = env->FindClass("com/Spotify/Album");
	jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I" );
	int nativePtr = env->GetIntField( object, fid );
	
	Spotify::JAlbum* pAlbum = reinterpret_cast<Spotify::JAlbum*>( Spotify::NativePtrToPointer( nativePtr ) );

	return pAlbum;
}

JNIEXPORT jstring JNICALL Java_Spotify_Album_GetName
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();	

	Spotify::JAlbum* pAlbum = GetAlbumNativePtr( env, object );

	jstring jstr = env->NewStringUTF( pAlbum->GetName().c_str() );

	return jstr;
}

JNIEXPORT jboolean JNICALL Java_Spotify_Album_IsLoading
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();	

	Spotify::JAlbum* pAlbum = GetAlbumNativePtr( env, object );

	return pAlbum->IsLoading();
}

JNIEXPORT jobject JNICALL Java_Spotify_Album_GetImage
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();	

	Spotify::JAlbum* pAlbum = GetAlbumNativePtr( env, object );

	Spotify::JImage* pImage = static_cast<Spotify::JImage*>( pAlbum->GetImage() );

	if (pImage == NULL)
	{
		return NULL;
	}

	jclass cls = env->FindClass( "com/Spotify/Image" );
	jmethodID cid = env->GetMethodID( cls, "<init>", "(I)V");
		
	jobject javaObject = env->NewObject( cls, cid, PointerToNativePtr(pImage) );

	return javaObject;
}

JNIEXPORT void JNICALL Java_Spotify_Album_Release
  (JNIEnv *env, jobject object)
{
	Spotify::JAlbum* pAlbum = GetAlbumNativePtr( env, object );
	
	pAlbum->ThreadSafeRelease();

	jclass cls = env->FindClass("com/Spotify/Album");
	jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I" );
	env->SetIntField( object, fid, 0 );
}

namespace Spotify
{
	void JAlbum::ThreadSafeRelease()
	{
		static_cast<JSession*>(m_pSession)->ThreadSafeRelease( this );
	}
}
