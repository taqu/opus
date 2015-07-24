
LOCAL_PATH := $(call my-dir)


SRC := ../../../../..
INCDIR := $(LOCAL_PATH)/../../../../..

ifeq ($(MODE), DEBUG)
CPPFLAGS := -D_DEBUG
endif
ifeq ($(LOGENABLE), ENABLE)
CPPFLAGS += -DLIME_ENABLE_LOG
endif

C_INCLUDES := $(INCDIR) $(INCDIR)/ext/include $(INCDIR)/ext/include/opus

EXTLIB_DIR := $(LOCAL_PATH)/../../../../../ext/lib
LDFLAGS := -L$(EXTLIB_DIR)

include $(CLEAR_VARS)
LOCAL_MODULE    := ogg
LOCAL_SRC_FILES := $(EXTLIB_DIR)/libogg.a
LOCAL_EXPORT_C_INCLUDES := $(C_INCLUDES)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := opus
LOCAL_SRC_FILES := $(EXTLIB_DIR)/libopus.a
LOCAL_EXPORT_C_INCLUDES := $(C_INCLUDES)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := opusfile
LOCAL_SRC_FILES := $(EXTLIB_DIR)/libopusfile.a
LOCAL_EXPORT_C_INCLUDES := $(C_INCLUDES)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lcore
LOCAL_CPPFLAGS += $(CPPFLAGS)
LOCAL_C_INCLUDES := $(C_INCLUDES)
LOCAL_SRC_FILES :=\
	$(SRC)/lcore/lcore.cpp\
	$(SRC)/lcore/liostream.cpp\
	$(SRC)/lcore/async/SyncObject.cpp\
	$(SRC)/lcore/async/Thread.cpp\

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lsound
LOCAL_CPPFLAGS += $(CPPFLAGS)
LOCAL_C_INCLUDES := $(C_INCLUDES)
LOCAL_SRC_FILES :=\
	$(SRC)/lsound/opus/Pack.cpp\
	$(SRC)/lsound/opus/PackReader.cpp\
	$(SRC)/lsound/opus/Resource.cpp\
	$(SRC)/lsound/opus/Stream.cpp\
	$(SRC)/lsound/OpenSL/Context.cpp\
	$(SRC)/lsound/OpenSL/Player.cpp\
	$(SRC)/lsound/OpenSL/UserPlayer.cpp\


include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio
LOCAL_CPPFLAGS += $(CPPFLAGS)
LOCAL_C_INCLUDES := $(C_INCLUDES)
LOCAL_LDFLAGS += $(LDFLAGS)
LOCAL_LDLIBS += -lOpenSLES -llog -landroid
LOCAL_SRC_FILES :=\
	audio.cpp\

LOCAL_STATIC_LIBRARIES := lsound lcore opusfile ogg opus
include $(BUILD_SHARED_LIBRARY)

