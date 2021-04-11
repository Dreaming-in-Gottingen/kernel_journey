LOCAL_PATH:= $(call my-dir)

########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := ApiTester.cpp

LOCAL_SHARED_LIBRARIES := libcutils   \
                          libutils    \
                          liblog      \

LOCAL_MODULE := ApiTester
include $(BUILD_EXECUTABLE)
