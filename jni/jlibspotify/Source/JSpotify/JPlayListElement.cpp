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

// c lib
#include <assert.h>

// local
#include "JPlayListElement.h"
#include "JUtils.h"
#include "JSession.h"

// JNI
#include "Spotify/Spotify_PlayListElement.h"

// LibSpotify++
#include "Spotify/PlayListElement.h"

#define PARAONIA_PRINTF(msg, ...)	//printf(msg,__VA_ARGS__)

JNIEXPORT jobject JNICALL Java_Spotify_PlayListElement_GetParent
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
	
	Spotify::PlayListElement* pParent = pElement->GetParent();
	if (pParent)
	{
		Spotify::JPlayListElement* pJParent = reinterpret_cast<Spotify::JPlayListElement*>( pParent->GetUserData() );
	}

	return NULL;
}

JNIEXPORT jboolean JNICALL Java_Spotify_PlayListElement_HasChildren
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
	return pElement->HasChildren();
}

JNIEXPORT jint JNICALL Java_Spotify_PlayListElement_GetNumChildren
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
	int numChildren = pElement->GetNumChildren();
	return numChildren;
}

JNIEXPORT jobject JNICALL Java_Spotify_PlayListElement_GetChild
  (JNIEnv *env, jobject javaObject, jint index)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
		
	Spotify::PlayListElement* pChild = pElement->GetChild( index );
		
	if (pChild)
	{	
		Spotify::JPlayListElement* pJChild = reinterpret_cast<Spotify::JPlayListElement*>( pChild->GetUserData() );		
		jobject jChild = pJChild->GetJavaObject();
				
		PARAONIA_PRINTF("GetChild[%d] pElement [0x%08X] pChild [0x%08X] pJChild [0x%08X] jChild [0x%08X]\n", int(index), pElement, pChild, pJChild, jChild );

		return jChild;
	}

	return NULL;

}

JNIEXPORT jboolean JNICALL Java_Spotify_PlayListElement_IsLoading
  (JNIEnv *env, jobject javaObject, jboolean recursive )
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
	bool isLoading = pElement->IsLoading( recursive == JNI_TRUE );
	return isLoading;
}

JNIEXPORT jstring JNICALL Java_Spotify_PlayListElement_GetName
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );

	jstring jstr = env->NewStringUTF( pElement->GetName().c_str() );
	return jstr;
}

JNIEXPORT jint JNICALL Java_Spotify_PlayListElement_GetType
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );
	return pElement->GetType();
}

JNIEXPORT void JNICALL Java_Spotify_PlayListElement_Release
  (JNIEnv *env, jobject javaObject)
{
	Spotify::PlayListElement* pElement = Spotify::JPlayListElement::GetPlayListElement( env, javaObject );

	if (pElement)
	{
		Spotify::JPlayListElement* pJElement = reinterpret_cast<Spotify::JPlayListElement*>( pElement->GetUserData() );
	
		pJElement->ReleaseJavaObject( pElement, javaObject );
	
		static_cast<Spotify::JSession*>(pElement->GetSession())->ThreadSafeRelease( pElement );
	}
}

namespace Spotify
{
	JPlayListElement::~JPlayListElement()
	{
		JSESSION_VALIDATE_THREAD();	

		if (m_javaObject != 0)
		{
			m_env->DeleteGlobalRef( m_javaObject );
			m_env = NULL;
			m_javaObject = NULL;
		}
	}

	jobject Spotify::JPlayListElement::GetJavaObject()
	{
		return m_javaObject;
	}

	void Spotify::JPlayListElement::InitialiseJavaObject(JNIEnv *env, PlayListElement* pElement, const char* javaClassName )
	{
		JSESSION_VALIDATE_THREAD();	

		m_env = env;

		PARAONIA_PRINTF("InitialiseJavaObject() [0x%08X]\n", this);
		
		PARAONIA_PRINTF("- env [0x%08X] pElement [0x%08X] javaClassName [%s]\n", env, pElement, javaClassName );
		
		// store a ptr to this JPlayListElement interface in pElement
		pElement->SetUserData( this );
					
		// create java object
		jclass cls = env->FindClass( javaClassName );
		jmethodID cid = env->GetMethodID( cls, "<init>", "(I)V");
		
		jobject javaObject = m_env->NewObject( cls, cid, PointerToNativePtr(pElement) );
		
		// store a global ref to the java object		
		m_javaObject = m_env->NewGlobalRef( javaObject );

		PARAONIA_PRINTF("- javaObject [0x%08X] m_javaObject [0x%08X] cls [0x%08X] cid [0x%08X]\n", javaObject, m_javaObject, cls, cid);
	}

	PlayListElement* Spotify::JPlayListElement::GetPlayListElement(JNIEnv *env, jobject javaObject)
	{
		jclass cls = env->FindClass("com/Spotify/PlayListElement");
		jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I" );
		int nativePtr = env->GetIntField( javaObject, fid );
	
		Spotify::PlayListElement* pElement = reinterpret_cast<Spotify::PlayListElement*>( Spotify::NativePtrToPointer( nativePtr ) );

		PARAONIA_PRINTF("GetPlayListElement() pElement [0x%08X] javaObject [0x%08X]\n", pElement, javaObject);
		return pElement;
	}

	void Spotify::JPlayListElement::ReleaseJavaObject( PlayListElement* pElement, jobject javaObject )
	{		
		assert( m_env->IsSameObject( javaObject, m_javaObject ) );

		if (m_javaObject)
		{
			pElement->SetUserData( NULL );

			// clear the nativePtr value in java 
			jclass cls = m_env->FindClass("com/Spotify/PlayListElement");
			jfieldID fid = m_env->GetFieldID( cls, "m_nativePtr", "I");
			m_env->SetIntField( javaObject, fid, 0 );
		
			m_env->DeleteGlobalRef( m_javaObject );

			m_javaObject = NULL;
			m_env = NULL;
		}
	}
}
