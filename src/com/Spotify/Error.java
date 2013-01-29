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

package com.Spotify;

public class Error {

	public final int SP_ERROR_OK                        = 0;  ///< No errors encountered
	public final int SP_ERROR_BAD_API_VERSION           = 1;  ///< The library version targeted does not match the one you claim you support
	public final int SP_ERROR_API_INITIALIZATION_FAILED = 2;  ///< Initialization of library failed - are cache locations etc. valid?
	public final int SP_ERROR_TRACK_NOT_PLAYABLE        = 3;  ///< The track specified for playing cannot be played
	public final int SP_ERROR_RESOURCE_NOT_LOADED       = 4;  ///< One or several of the supplied resources is not yet loaded
	public final int SP_ERROR_BAD_APPLICATION_KEY       = 5;  ///< The application key is invalid
	public final int SP_ERROR_BAD_USERNAME_OR_PASSWORD  = 6;  ///< Login failed because of bad username and/or password
	public final int SP_ERROR_USER_BANNED               = 7;  ///< The specified username is banned
	public final int SP_ERROR_UNABLE_TO_CONTACT_SERVER  = 8;  ///< Cannot connect to the Spotify backend system
	public final int SP_ERROR_CLIENT_TOO_OLD            = 9;  ///< Client is too old; library will need to be updated
	public final int SP_ERROR_OTHER_PERMANENT           = 10; ///< Some other error occured; and it is permanent (e.g. trying to relogin will not help)
	public final int SP_ERROR_BAD_USER_AGENT            = 11; ///< The user agent string is invalid or too long
	public final int SP_ERROR_MISSING_CALLBACK          = 12; ///< No valid callback registered to handle events
	public final int SP_ERROR_INVALID_INDATA            = 13; ///< Input data was either missing or invalid
	public final int SP_ERROR_INDEX_OUT_OF_RANGE        = 14; ///< Index out of range
	public final int SP_ERROR_USER_NEEDS_PREMIUM        = 15; ///< The specified user needs a premium account
	public final int SP_ERROR_OTHER_TRANSIENT           = 16; ///< A transient error occured.
	public final int SP_ERROR_IS_LOADING                = 17; ///< The resource is currently loading
	public final int SP_ERROR_NO_STREAM_AVAILABLE       = 18; ///< Could not find any suitable stream to play
	public final int SP_ERROR_PERMISSION_DENIED         = 19; ///< Requested operation is not allowed
	public final int SP_ERROR_INBOX_IS_FULL             = 20; ///< Target inbox is full
}
