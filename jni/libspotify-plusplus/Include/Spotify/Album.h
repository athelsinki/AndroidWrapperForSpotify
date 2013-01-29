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

// std include
#include <string>
#include <stdio.h>

// libspotify include
#include "libspotify/api.h"

namespace Spotify
{
	// forward declarations
	class Image;
	class Session;

	class Album
	{
	public:

		Album( Session* pSession );
		virtual ~Album();

		virtual void Load( sp_album* pAlbum );

		virtual bool IsLoading();

		virtual std::string GetName();

		virtual Image* GetImage();		

	protected:

		sp_album*		m_pAlbum;
		Session*		m_pSession;
	};

}
