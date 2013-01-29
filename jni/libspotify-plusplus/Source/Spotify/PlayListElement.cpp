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

#include "Spotify/PlayListElement.h"
#include "Spotify/Session.h"


namespace Spotify
{	
	PlayListElement::PlayListElement( Session* pSession ) : m_pParent(NULL), m_pSession(pSession), m_pUserData(NULL)
	{
	}

	PlayListElement::~PlayListElement()
	{
	}

	PlayListElement* PlayListElement::GetParent() const
	{
		return m_pParent;
	}

	void PlayListElement::SetParent( PlayListElement* pParent )
	{
		m_pParent = pParent;
	}

	void* PlayListElement::GetUserData()
	{
		return m_pUserData;
	}

	void PlayListElement::SetUserData( void* pUserData )
	{
		m_pUserData = pUserData;
	}
	
	Session* PlayListElement::GetSession()
	{
		return m_pSession;
	}
	
} // spotify
