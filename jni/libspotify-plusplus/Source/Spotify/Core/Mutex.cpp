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

#include "Spotify/Core/Mutex.h"

#include <android/log.h>
#define LOG_TAG "MBS"

namespace Spotify
{
	namespace Core
	{
		Mutex::Mutex()
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Mutex");
			theMutex = new pthread_mutex_t();
			int mutexInt = pthread_mutex_init( theMutex, NULL );
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Mutex:end:mutexInt:%d, theMutex:%x", mutexInt, theMutex);
		}

		Mutex::~Mutex()
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:~Mutex");
			pthread_mutex_destroy( theMutex );
		}


		int Mutex::TryLock()
		{
			return pthread_mutex_trylock( theMutex );
		}

		int Mutex::Lock()
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Lock:%x", theMutex);
			return pthread_mutex_lock( theMutex );
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Lock:end");
		}

		int Mutex::Unlock()
		{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Unlock:%x", theMutex);
			return pthread_mutex_unlock( theMutex );
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "--- Mutex:Unlock:end");
		}


		ScopedLock::ScopedLock( Mutex* pMutex )
		{
			m_pMutex = pMutex;
			m_pMutex->Lock();
		}
	
		ScopedLock::~ScopedLock()
		{
			m_pMutex->Unlock();
			m_pMutex = NULL;
		}
	} // Core
} // Spotify
