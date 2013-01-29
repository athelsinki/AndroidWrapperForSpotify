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

public class Album {

	public native String GetName();
	
	public native boolean IsLoading();
	
	public native Image GetImage();
	
	public void finalize()
	{
		Release();
	}
	
	public native void Release();	
	
	private Album( int nativePtr )
	{
		m_nativePtr = nativePtr;
	}
	
	int m_nativePtr = 0;	
}
