LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := api_test.ko
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

$(LOCAL_PATH)/api_test.ko: $(TARGET_PREBUILT_KERNEL)
	$(MAKE) -C $(shell dirname $@) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) KDIR=$(ANDROID_PRODUCT_OUT)/obj/KERNEL clean
	$(MAKE) -C $(shell dirname $@) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) KDIR=$(ANDROID_PRODUCT_OUT)/obj/KERNEL
	$(TARGET_STRIP) -d --strip-unneeded $@
