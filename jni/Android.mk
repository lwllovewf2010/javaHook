LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=  inject.c libinject.c shellcode.s
LOCAL_MODULE := inject
LOCAL_LDLIBS := \
	-llog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
	DalvikMethodHook.cpp \
	libtest.cpp
LOCAL_MODULE := test
LOCAL_LDLIBS := -llog -ldvm.4 -landroid_runtime
LOCAL_CFLAGS    := -DDEBUG -I./jni/include/ 
LOCAL_LDFLAGS	:=	-L./jni/lib/

include $(BUILD_SHARED_LIBRARY)
