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

#include "JSession.h"
#include "JTrack.h"
#include "JPlayList.h"
#include "JPlayListContainer.h"
#include "JPlayListFolder.h"
#include "JArtist.h"
#include "JAlbum.h"
#include "JImage.h"


#include "JUtils.h"

#include "Spotify/Spotify_Session.h"
#include "Spotify/Spotify_Session_Config.h"

#ifdef THREADING_CHECKS
//#include <Windows.h>
#include <assert.h>

unsigned int Spotify::JSession::ms_threadID;
#endif

#define LOG(msg,...)	//
#define LOG_TAG "MBS"

#include <android/log.h>
#include <stdarg.h>

#if ECLIPSEBUILD // this part is just to fix spurious Eclipse errors
    typedef __builtin_va_list va_list;
    #define va_start(v,l)   __builtin_va_start(v,l)
    #define va_end(v)   __builtin_va_end(v)
    #define va_arg(v,l) __builtin_va_arg(v,l)
    #if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__GXX_EXPERIMENTAL_CXX0X__)
    #define va_copy(d,s)    __builtin_va_copy(d,s)
    #endif
#endif
#include <stdlib.h>

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm2, void* reserved)
{
	JSESSION_VALIDATE_THREAD();

	JNIEnv* env = NULL;
	jint result = -1;

	if(JNI_OK != vm2->GetEnv((void **)&env, JNI_VERSION_1_6))
	{
		return -1;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_6;

	return result;
}

JNIEXPORT jint JNICALL Java_com_Spotify_Session_NativeCreate
  (JNIEnv *env, jobject object)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:NativeCreate");

	Spotify::JSession* pSession = new Spotify::JSession( env, object );
		
	return Spotify::PointerToNativePtr( pSession );
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_NativeDestroy
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	delete pSession;
}

JNIEXPORT jint JNICALL Java_com_Spotify_Session_Initialise
  (JNIEnv *env, jobject object, jint nativePtr, jobject jconfig)
{

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:initialise");

	JSESSION_VALIDATE_THREAD();

	jboolean isCopy;
	jfieldID fid;

	jclass cls = env->GetObjectClass(jconfig);
	
	// appkey
	fid = env->GetFieldID( cls, "m_appKey", "[C" );
	jobject appKeyObj = env->GetObjectField( jconfig, fid );
	jcharArray appKeyCharArray = static_cast<jcharArray>(appKeyObj);
	jchar* appKey = env->GetCharArrayElements( appKeyCharArray, &isCopy );

	fid = env->GetFieldID( cls, "m_appKeySize", "I" );
	jint appKeySize = env->GetIntField( jconfig, fid );

	// make local binary copy of key
	uint8_t* localAppKey = new uint8_t[ appKeySize ];
	for (int i=0; i<appKeySize; i++)
	{
		localAppKey[i] = (uint8_t) appKey[i];
	}

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:initialise:appKeySize:%d", appKeySize);

	fid = env->GetFieldID( cls, "m_cacheLocation", "Ljava/lang/String;" );	
	jstring strCacheLocation = static_cast<jstring>( env->GetObjectField( jconfig, fid ) );
	const char* szCacheLocation = env->GetStringUTFChars(strCacheLocation, &isCopy );

	fid = env->GetFieldID( cls, "m_settingsLocation", "Ljava/lang/String;" );	
	jstring strSettingsLocation = static_cast<jstring>( env->GetObjectField( jconfig, fid ) );
	const char* szSettingsLocation = env->GetStringUTFChars(strSettingsLocation, &isCopy );

	//
	//			bool			m_compressPlaylists;
	//			bool			m_dontSaveMetadataForPlaylists;
	//			bool			m_initiallyUnloadPlaylists;
	/*
	fid = env->GetFieldID( cls, "m_tinySettings", "Z" );
	jboolean tinySettings = env->GetBooleanField( jconfig, fid );

	fid = env->GetFieldID( cls, "m_tinySettings", "Z" );
	jboolean tinySettings = env->GetBooleanField( jconfig, fid );

	fid = env->GetFieldID( cls, "m_tinySettings", "Z" );
	jboolean tinySettings = env->GetBooleanField( jconfig, fid );
	*/

	fid = env->GetFieldID( cls, "m_userAgent", "Ljava/lang/String;" );	
	jstring strUserAgent = static_cast<jstring>( env->GetObjectField( jconfig, fid ) );
	const char* szUserAgent = env->GetStringUTFChars(strUserAgent, &isCopy );
  	
	Spotify::JSession::Config config;
	config.m_appKey = localAppKey;
	config.m_appKeySize = appKeySize;
	config.m_cacheLocation = szCacheLocation;
	config.m_settingsLocation = szSettingsLocation;
	config.m_userAgent = szUserAgent;

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:initialise:m_appKey:%s", config.m_appKey);

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	sp_error error = pSession->Initialise( config );
	
	env->ReleaseStringUTFChars(strCacheLocation, szCacheLocation);
	env->ReleaseStringUTFChars(strSettingsLocation, szSettingsLocation);
	env->ReleaseStringUTFChars(strUserAgent, szUserAgent);

	delete [] localAppKey;
	env->ReleaseCharArrayElements( appKeyCharArray, appKey, 0 );
	
	//create a track
	pSession->pTrack = pSession->CreateTrack();

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:initialise:end:%d", error);

	return error;
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Shutdown
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Shutdown();		
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Update
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Update();
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Login
  (JNIEnv *env, jobject object, jint nativePtr, jstring username, jstring password)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Jsession:login");

	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	jboolean isCopy;
	const char* szUsername = env->GetStringUTFChars(username, &isCopy );
	const char* szPassword = env->GetStringUTFChars(password, &isCopy );

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Jsession:login:%s,%s", szUsername, szPassword);

	pSession->Login( szUsername, szPassword );

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### Jsession:login:end");

	env->ReleaseStringUTFChars( username, szUsername );
	env->ReleaseStringUTFChars( password, szPassword );
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Logout
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Logout();
}

JNIEXPORT jboolean JNICALL Java_com_Spotify_Session_IsLoggedIn
  (JNIEnv *env, jobject object, jint nativePtr)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:IsLoggedIn");

	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:IsLoggedIn:end");
	return pSession->IsLoggedIn();
}

JNIEXPORT jboolean JNICALL Java_com_Spotify_Session_IsLoaded
  (JNIEnv *env, jobject object, jint nativePtr, jboolean recursive)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:IsLoaded");

	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	bool loaded = pSession->pTrack->IsLoading(recursive);

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:IsLoaded:end:Loaded:%d",loaded);

	return loaded;
}

JNIEXPORT jint JNICALL Java_com_Spotify_Session_GetConnectionState
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	return pSession->GetConnectionState();
}

static Spotify::Track* GetTrackFromNativePtr( JNIEnv* env, jobject track )
{
	JSESSION_VALIDATE_THREAD();

	// get Track ptr from track->m_nativePtr
	jclass cls = env->FindClass("com/Spotify/Track");
	jfieldID fid = env->GetFieldID( cls, "m_nativePtr", "I");
	
	jint nativePtr = env->GetIntField( track, fid );
	Spotify::PlayListElement* pElement = reinterpret_cast< Spotify::PlayListElement* >( Spotify::NativePtrToPointer( nativePtr ) );

	return static_cast< Spotify::Track* >( pElement );
}

JNIEXPORT jint JNICALL Java_com_Spotify_Session_Load
  (JNIEnv *env, jobject object, jint nativePtr, jstring trackid)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	//Spotify::Track* pTrack = GetTrackFromNativePtr( env, track );

	jboolean isCopy;
	const char* szTrackid = env->GetStringUTFChars(trackid, &isCopy );

	pSession->pTrack->Load(szTrackid);

	return pSession->Load( pSession->pTrack );
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Unload
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	
	pSession->Unload( pSession->pTrack);
	pSession->pTrack->Unload();
}

JNIEXPORT jobject JNICALL Java_com_Spotify_Session_GetCurrentTrack
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	Spotify::Track* pTrack = pSession->GetCurrentTrack();

	if (pTrack)
	{
		Spotify::JTrack* pJTrack = static_cast<Spotify::JTrack*>( pTrack );
		jobject jtrack = pJTrack->GetJavaObject();
		
		return jtrack;
	}

	return NULL;
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Seek
  (JNIEnv *env, jobject object, jint nativePtr, jint offset)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Seek( offset );
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Play
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Play(pSession->pTrack);
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_Stop
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	pSession->Stop();
}

JNIEXPORT jint JNICALL Java_com_Spotify_Session_PreFetch
  (JNIEnv *env, jobject object, jint nativePtr, jobject track)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	Spotify::Track* pTrack = GetTrackFromNativePtr( env, track );

	sp_error error = pSession->PreFetch( pTrack );
	return error;
}

JNIEXPORT jobject JNICALL Java_com_Spotify_Session_GetPlayListContainer
  (JNIEnv *env, jobject object, jint nativePtr)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );

	Spotify::JPlayListContainer* pPlayListContainer = static_cast<Spotify::JPlayListContainer*>( pSession->GetPlayListContainer() );

	if (pPlayListContainer)
	{
		jobject jPlayListContainer = pPlayListContainer->GetJavaObject();

		return jPlayListContainer;
	}
	else
	{
		return NULL;
	}
}

JNIEXPORT void JNICALL Java_com_Spotify_Session_SetPreferredBitrate
  (JNIEnv *env, jobject object, jint nativePtr, jint bitrate)
{
	JSESSION_VALIDATE_THREAD();

	Spotify::JSession* pSession = reinterpret_cast< Spotify::JSession* >( Spotify::NativePtrToPointer( nativePtr ) );
	pSession->SetPreferredBitrate( static_cast<sp_bitrate>(bitrate) );
}

namespace Spotify
{
	JSession::JSession(JNIEnv *env, jobject session)
	{
#ifdef THREADING_CHECKS
		ms_threadID = GetCurrentThreadId();		
#endif
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:JSession()");

		m_env = env;
		m_env->GetJavaVM( &vm );
		m_session = m_env->NewGlobalRef( session );

		jclass localclsAlbum = env->FindClass("com/Spotify/Album");
		clsAlbum  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsAlbum));
		jclass localclsArtist = env->FindClass("com/Spotify/Artist");
		clsArtist  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsArtist));
		jclass localclsAudioBufferStats = env->FindClass("com/Spotify/AudioBufferStats");
		clsAudioBufferStats  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsAudioBufferStats));
		jclass localclsAudioFormat = env->FindClass("com/Spotify/AudioFormat");
		clsAudioFormat  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsAudioFormat));
		jclass localclsImage = env->FindClass("com/Spotify/Image");
		clsImage  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsImage));
		jclass localclsPlayList = env->FindClass("com/Spotify/PlayList");
		clsPlayList  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsPlayList));
		jclass localclsPlayListContainer = env->FindClass("com/Spotify/PlayListContainer");
		clsPlayListContainer  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsPlayListContainer));
		jclass localclsPlayListElement = env->FindClass("com/Spotify/PlayListElement");
		clsPlayListElement  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsPlayListElement));
		jclass localclsPlayListFolder = env->FindClass("com/Spotify/PlayListFolder");
		clsPlayListFolder  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsPlayListFolder));
		jclass localclsTrack = env->FindClass("com/Spotify/Track");
		clsTrack  = reinterpret_cast<jclass>(env->NewGlobalRef(localclsTrack));

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "### JSession:JSession():end");
	}

	JSession::~JSession()
	{
		JSESSION_VALIDATE_THREAD();

		m_env->DeleteGlobalRef( m_session );
		m_session = NULL;
		m_env->DeleteGlobalRef( clsAlbum );
		clsAlbum = NULL;
		m_env->DeleteGlobalRef( clsArtist );
		clsArtist = NULL;
		m_env->DeleteGlobalRef( clsAudioBufferStats );
		clsAudioBufferStats = NULL;
		m_env->DeleteGlobalRef( clsAudioFormat );
		clsAudioFormat = NULL;
		m_env->DeleteGlobalRef( clsImage );
		clsImage = NULL;
		m_env->DeleteGlobalRef( clsPlayList );
		clsPlayList = NULL;
		m_env->DeleteGlobalRef( clsPlayListContainer );
		clsPlayListContainer = NULL;
		m_env->DeleteGlobalRef( clsPlayListElement );
		clsPlayListElement = NULL;
		m_env->DeleteGlobalRef( clsPlayListFolder );
		clsPlayListFolder = NULL;
		m_env->DeleteGlobalRef( clsTrack );
		clsTrack = NULL;
	}

	PlayList* JSession::CreatePlayList()
	{
		JSESSION_VALIDATE_THREAD();

		return new JPlayList( this );
	}

	PlayListContainer* JSession::CreatePlayListContainer()
	{
		JSESSION_VALIDATE_THREAD();

		return new JPlayListContainer( this );
	}

	PlayListFolder* JSession::CreatePlayListFolder()
	{
		JSESSION_VALIDATE_THREAD();

		return new JPlayListFolder( this );
	}

	Track* JSession::CreateTrack()
	{
		JSESSION_VALIDATE_THREAD();

		return new JTrack( this );
	}

	Artist* JSession::CreateArtist()
	{
		JSESSION_VALIDATE_THREAD();

		return new JArtist( this );
	}

	Album* JSession::CreateAlbum()
	{
		JSESSION_VALIDATE_THREAD();

		return new JAlbum( this );
	}

	Image* JSession::CreateImage()
	{
		JSESSION_VALIDATE_THREAD();

		return new JImage( this );
	}

	void JSession::Update()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** JSession:Update");

		JSESSION_VALIDATE_THREAD();

		// perform any cleanup required

		Core::ScopedLock autoLock( &m_threadSafeReleaseMutex );

		while (!m_threadSafeReleaseJobs.empty())
		{
			ReleaseJobBase* pJob = m_threadSafeReleaseJobs.front();
			m_threadSafeReleaseJobs.pop_front();

			pJob->Release();
			delete pJob;
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** JSession::Update:middle");

		// call Update() in the super class
		Session::Update();

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "***** JSession:Update:end");
	}

	void JSession::JNICallVoidMethod( JNIEnv* env, const char* name, const char* sig, ... )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod");

		jclass cls;

		if (env != NULL)
		{
			cls = env->GetObjectClass(m_session);
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod:env->GetObjectClass");
		}



		if (cls != 0)
		{
			jmethodID mid = env->GetMethodID( cls, name, sig );
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod:env->GetMethodID");
			if (mid != 0)
			{		
				va_list args;
				va_start (args, sig);		
				env->CallVoidMethodV( m_session, mid, args );
				__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod:env->CallVoidMethodV");
				va_end(args);	
			}
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod:end");
	}

	void JSession::OnLoggedIn( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn");
		Session::OnLoggedIn( error );

		LOG("OnLoggedIn");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnLoggedIn", "(I)V", jint(error) );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedIn:end");
	}

	void JSession::OnLoggedOut()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut");

		Session::OnLoggedOut();
		LOG("OnLoggedOut");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut: vm->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnLoggedOut", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLoggedOut:end");
	}

	void JSession::OnMetadataUpdated()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated");

		Session::OnMetadataUpdated();
		LOG("OnMetadataUpdated");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated: vm->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnMetadataUpdated", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMetadataUpdated:end");
	}

	void JSession::OnConnectionError( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError");

		Session::OnConnectionError( error );
		LOG("OnConnectionError");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:env: OnConnectionError->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnConnectionError", "(I)V", jint(error) );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnConnectionError:end");
	}

	void JSession::OnMessageToUser( const char* message )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession::OnMessageToUser");
		Session::OnMessageToUser( message );

		LOG("OnMessageToUser");
		LOG( message );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser: AttachCurrentThread");
		}

		jstring jstr = env->NewStringUTF( message );
		JNICallVoidMethod( env, "OnLoggedOut", "(Ljava/lang/String;)V", jstr );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMessageToUser:end");
	}

	void JSession::OnNotifyMainThread()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread");

		Session::OnNotifyMainThread();

		LOG("OnNotifyMainThread");
		
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread:middle");


		JNIEnv* env = NULL;
		bool isAttached = false;


		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread:vm->GetEnv");


		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnNotifyMainThread", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnNotifyMainThread:end");
	}

	/*
	int  JSession::OnMusicDelivery( const sp_audioformat* format, const void* frames, int num_frames )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMusicDelivery");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnMusicDelivery:end");
		
		return Session::OnMusicDelivery(format, frames, num_frames);
	}*/

	void JSession::OnPlayTokenLost()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost");

		Session::OnPlayTokenLost();
		LOG("OnPlayTokenLost");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnPlayTokenLost", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnPlayTokenLost:end");
	}

	void JSession::OnLogMessage( const char* data )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage");
		Session::OnLogMessage( data );

		LOG("OnLogMessage");
		LOG( data );

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:JNICallVoidMethod: vm->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage: AttachCurrentThread");
		}

		jstring jstr = env->NewStringUTF( data );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage:env->NewStringUTF");
		JNICallVoidMethod( env, "OnLogMessage", "(Ljava/lang/String;)V", jstr );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnLogMessage:end");
	}

	void JSession::OnEndOfTrack()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack");

		Session::OnEndOfTrack();

		LOG("OnEndOfTrack");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnEndOfTrack", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnEndOfTrack:end");
	}

	void JSession::OnStreamingError( sp_error error )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError");

		Session::OnStreamingError( error );
		LOG("OnStreamingError");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnStreamingError", "(I)V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStreamingError:end");
	}

	void JSession::OnUserinfoUpdated()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated");

		Session::OnUserinfoUpdated();
		LOG("OnUserinfoUpdated");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated: vm->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated: AttachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated:before JNICallVoidMethod");
		JNICallVoidMethod( env, "OnUserinfoUpdated", "()V" );
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated:after JNICallVoidMethod");

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnUserinfoUpdated:DetachCurrentThread");
		}

	}

	void JSession::OnStartPlayback()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback");

		Session::OnStartPlayback();
		LOG("OnStartPlayback");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnStartPlayback", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStartPlayback:end");
	}

	void JSession::OnStopPlayback()
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback");

		Session::OnStopPlayback();
		LOG("OnStopPlayback");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback: AttachCurrentThread");
		}

		JNICallVoidMethod( env, "OnStopPlayback", "()V" );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnStopPlayback:end");
	}

	void JSession::OnGetAudioBufferStats( sp_audio_buffer_stats* stats )
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats");

		Session::OnGetAudioBufferStats( stats );
		LOG("OnGetAudioBufferStats");

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats:middle");

		JNIEnv* env = NULL;
		bool isAttached = false;

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats: env->GetEnv");

		if(JNI_OK == vm->GetEnv((void **)&env, JNI_VERSION_1_6))
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats: thread attached already, vm->GetEnv()");
		}
		else
		{
			vm->AttachCurrentThread(&env, NULL );
			isAttached = true;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats: AttachCurrentThread");
		}

		jmethodID cid = env->GetMethodID(clsAudioBufferStats, "<init>", "()V");
		jobject jstats = env->NewObject( clsAudioBufferStats, cid );

		jfieldID samplesID = env->GetFieldID( clsAudioBufferStats, "m_samples", "I");
		jfieldID stutterID = env->GetFieldID( clsAudioBufferStats, "m_stutter", "I");

		env->SetIntField(jstats, samplesID, stats->samples);
		env->SetIntField(jstats, stutterID, stats->stutter);

		JNICallVoidMethod( env, "OnGetAudioBufferStats", "(Lcom/Spotify/AudioBufferStats;)V", jstats );

		if(isAttached)
		{
			vm->DetachCurrentThread();
			isAttached = false;
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats:DetachCurrentThread");
		}

		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- JSession:OnGetAudioBufferStats:end");
	}

	JNIEnv* JSession::GetEnv()
	{
		return m_env;
	}

	Track* JSession::GetCurrentTrack()
	{
		return pTrack;
	}

#ifdef THREADING_CHECKS
	void Spotify::JSession::ValidateThread( const char* label )
	{
		if (ms_threadID != GetCurrentThreadId() )
		{
			printf("ValidateThread [%s] ms_threadID [%u] != current [%u]\n", label, ms_threadID, GetCurrentThreadId() );

			assert( ms_threadID == GetCurrentThreadId() );
		}
	}
#endif
}

