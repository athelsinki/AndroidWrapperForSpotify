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

public class PlayListElement {

	public native PlayListElement GetParent();

	public native boolean HasChildren();

	public native int GetNumChildren();

	public native PlayListElement GetChild(int index);

	public native boolean IsLoading( boolean recursive );

	public native String GetName();

	public static final int PLAYLIST = 0;
	public static final int PLAYLIST_FOLDER = 1;
	public static final int PLAYLIST_CONTAINER = 2;
	public static final int TRACK = 3;

	public native int GetType();

	public String GetTypeString() {
		int type = GetType();
		switch (type) {
		case PLAYLIST:
			return "PlayList";
		case PLAYLIST_FOLDER:
			return "PlayList Folder";
		case PLAYLIST_CONTAINER:
			return "PlayList Container";
		case TRACK:
			return "Track";
		default:
			return "UNKNOWN";
		}
	}

	public void InternalDumpToTTY(int level) {
		
		int numChildren = GetNumChildren();
		
		String str = new String();
		for (int i = 0; i < level; i++) {
			str += "  ";
		}

		str += GetTypeString();	
		
		str += " [" + GetName() + "] children " + numChildren ;

		System.out.println(str);

				
		level++;
				
		for (int i = 0; i < numChildren; i++) {
			PlayListElement pChild = GetChild(i);
			
			System.out.printf("CHILD CLASS TYPE [%s]\n", pChild.getClass().getName() );
				
			
			//System.out.printf( "child [%d]\n", pChild.GetNativePtr() );
				
			//System.out.printf(" child name [%s]\n", pChild.GetName());
			pChild.InternalDumpToTTY(level);
		}
	}

	public native void Release();
	
	public void finalize()
	{
		Release();
	}
	
	protected PlayListElement(int nativePtr) {
		m_nativePtr = nativePtr;
	}

	private int m_nativePtr = 0;
	public int GetNativePtr() {
		return m_nativePtr;
	}
}
