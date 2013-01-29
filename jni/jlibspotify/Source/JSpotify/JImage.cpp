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

#include "JImage.h"
#include "JUtils.h"
#include "JSession.h"

#include "Spotify/Spotify_Image.h"

static Spotify::JImage* GetImageNativePtr( JNIEnv* env, jobject object )
{
	jclass cls = env->FindClass("com/Spotify/Image");
	jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I" );
	int nativePtr = env->GetIntField( object, fid );
	
	Spotify::JImage* pImage = reinterpret_cast<Spotify::JImage*>( Spotify::NativePtrToPointer( nativePtr ) );

	return pImage;
}


JNIEXPORT jcharArray JNICALL Java_Spotify_Image_GetData
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();	

	Spotify::JImage* pImage = GetImageNativePtr( env, object );

	size_t numBytes;

	const void* pData = pImage->GetData( numBytes );

	jcharArray charArray = env->NewCharArray( numBytes );
		
	jboolean isCopy = false;
	jchar* pCharData = env->GetCharArrayElements( charArray, &isCopy );
	for (size_t i=0; i<numBytes; i++)
	{
		pCharData[i] = ( (byte*) pData)[i];
	}
		
	env->ReleaseCharArrayElements( charArray, pCharData, 0);

	return charArray;
}

JNIEXPORT jboolean JNICALL Java_Spotify_Image_IsLoading
  (JNIEnv *env, jobject object)
{
	JSESSION_VALIDATE_THREAD();	

	Spotify::JImage* pImage = GetImageNativePtr( env, object );

	return pImage->IsLoading();
}

JNIEXPORT void JNICALL Java_Spotify_Image_Release
  (JNIEnv *env, jobject object)
{
	Spotify::JImage* pImage = GetImageNativePtr( env, object );

	pImage->ThreadSafeRelease();

	jclass cls = env->FindClass("com/Spotify/Image");
	jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I" );
	env->SetIntField( object, fid, 0 );
}

namespace Spotify
{
	void JImage::ThreadSafeRelease()
	{
		static_cast<JSession*>(m_pSession)->ThreadSafeRelease( this );
	}
}

