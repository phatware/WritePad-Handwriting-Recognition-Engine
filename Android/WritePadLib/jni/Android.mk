LOCAL_PATH := $(call my-dir)

#########
### Static Library
#########

include $(CLEAR_VARS)

LOCAL_MODULE := libWritePadRecos

LOCAL_CFLAGS := -DRECODICT -D_EMBEDDED_DEVICE \
		-D_FLAT_32 -DHWR_SYSTEM=HWR_LINUX -D_OS_LINUX \
		-DLANG_SPANISH \
		-DLANG_SWED \
		-DLANG_GERMAN \
		-DLANG_FRENCH \
		-DLANG_DUTCH \
		-DLANG_PORTUGUESE \
		-DLANG_PORTUGUESEB \
		-DLANG_ITALIAN \
		-DLANG_NORW \
		-DLANG_DAN \
		-DLANG_FINNISH \
		-DLANG_ENGLISH \
		-DLANG_INDONESIAN \
		-DLANG_ENGLISHUK \
		-I. \
		-I$(LOCAL_PATH)/../../../UniversalRecognizer/V300/InkData \
		-I$(LOCAL_PATH)/../../../UniversalRecognizer/V300/RecognizerWrapper \
		-I$(LOCAL_PATH)/../../../UniversalRecognizer/V300/reco/include \
		-I$(LOCAL_PATH)/../../../UniversalRecognizer/V300/reco/data \
		-I$(LOCAL_PATH)/../../../UniversalRecognizer/V300/include \
		-Wno-multichar \
		-std=gnu++11
		

include jni/filelist.mk

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := WritePadReco

LOCAL_CFLAGS := -I$(LOCAL_PATH)/include -std=gnu99

LOCAL_SRC_FILES := interface.c letimg.c lidata.c

### LOCAL_LDLIBS := -ldl -llog # -lGLESv1_CM 
LOCAL_LDLIBS += -llog

LOCAL_STATIC_LIBRARIES := libWritePadRecos

include $(PREBUILD_SHARED_LIBRARY)

