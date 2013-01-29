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

// local includes
#include "Spotify/Album.h"
#include "Spotify/Image.h"
#include "Spotify/Session.h"

namespace Spotify
{
	Album::Album( Session* pSession )
	{
		m_pSession = pSession;
		m_pAlbum = NULL;
	}
	
	Album::~Album()
	{
		sp_album_release( m_pAlbum );
	}

	void Album::Load( sp_album* pAlbum )
	{
		m_pAlbum = pAlbum;
		sp_album_add_ref( m_pAlbum );
	}

	bool Album::IsLoading()
	{
		return !sp_album_is_loaded( m_pAlbum );
	}

	std::string Album::GetName()
	{
		return sp_album_name( m_pAlbum );
	}	

	Image* Album::GetImage()
	{
		if (IsLoading())
		{
			return NULL;
		}

		const byte* album_id = sp_album_cover(m_pAlbum,SP_IMAGE_SIZE_NORMAL);
		if (album_id == NULL)
		{
			return NULL;
		}

		Image* pImage = m_pSession->CreateImage();

		if (pImage->Load( album_id ))
		{
			return pImage;
		}
		else 
		{
			delete pImage;
			return NULL;
		}	
			
	}
}
