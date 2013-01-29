LOCAL_PATH := $(call my-dir)

#$(warning $(LOCAL_PATH))

include $(CLEAR_VARS)
LOCAL_MODULE := spotify
LOCAL_SRC_FILES := libspotify-12.1.51/lib/libspotify.so
LOCAL_CFLAGS := -g -Wno-psabi
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE  := spotify-plusplus
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/libspotify-plusplus/Include/ \
$(LOCAL_PATH)/libspotify-12.1.51/include/
LOCAL_SRC_FILES := \
libspotify-plusplus/Source/Spotify/Album.cpp \
libspotify-plusplus/Source/Spotify/Artist.cpp \
libspotify-plusplus/Source/Spotify/Core/Mutex.cpp \
libspotify-plusplus/Source/Spotify/Image.cpp \
libspotify-plusplus/Source/Spotify/PlayList.cpp \
libspotify-plusplus/Source/Spotify/PlayListContainer.cpp \
libspotify-plusplus/Source/Spotify/PlayListElement.cpp \
libspotify-plusplus/Source/Spotify/PlayListFolder.cpp \
libspotify-plusplus/Source/Spotify/Session.cpp \
libspotify-plusplus/Source/Spotify/Track.cpp \
libspotify-plusplus/Source/Spotify/sound_driver.cpp
LOCAL_LDLIBS := -llog -lOpenSLES
LOCAL_CPPFLAGS := -g -Wno-psabi
LOCAL_SHARED_LIBRARIES := spotify
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE  := JLibSpotify
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/libspotify-12.1.51/include/ \
$(LOCAL_PATH)/libspotify-plusplus/Include/ \
$(LOCAL_PATH)/jlibspotify/Include/ \
$(LOCAL_PATH)/jlibspotify/Source/JSpotify/
LOCAL_SRC_FILES := \
jlibspotify/Source/JSpotify/JAlbum.cpp \
jlibspotify/Source/JSpotify/JArtist.cpp \
jlibspotify/Source/JSpotify/JImage.cpp \
jlibspotify/Source/JSpotify/JPlayList.cpp \
jlibspotify/Source/JSpotify/JPlayListContainer.cpp \
jlibspotify/Source/JSpotify/JPlayListElement.cpp \
jlibspotify/Source/JSpotify/JPlayListFolder.cpp \
jlibspotify/Source/JSpotify/JSession.cpp \
jlibspotify/Source/JSpotify/JTrack.cpp
LOCAL_LDLIBS := -llog
LOCAL_CPPFLAGS := -g -Wno-psabi
LOCAL_SHARED_LIBRARIES := spotify-plusplus
include $(BUILD_SHARED_LIBRARY)