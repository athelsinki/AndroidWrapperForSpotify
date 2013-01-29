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

#include <jni.h>

#include "Spotify/PlayListElement.h"

namespace Spotify
{
	class JPlayListElement
	{
	public:
		virtual ~JPlayListElement();

		jobject		GetJavaObject();

		void		ReleaseJavaObject( PlayListElement* pElement, jobject javaObject );

		static		PlayListElement* GetPlayListElement(JNIEnv *env, jobject javaObject);

		void		ThreadSafeRelease();

	protected:
		void		InitialiseJavaObject( JNIEnv *env, PlayListElement* pElement, const char* javaClassName );		

	private:
		jobject		m_javaObject;
		JNIEnv*		m_env;
	};
}
