# Emulator product with sensors, location and battery served by our own HALs.
# @REMOVE_PACKAGES@ is substituted by `asp patch` (ASP_REMOVE_PACKAGES).
$(call inherit-product, device/generic/goldfish/64bitonly/product/sdk_phone64_x86_64.mk)
include vendor/andros/gen/andros_features.mk

PRODUCT_NAME := andros_x86_64
PRODUCT_MODEL := Android device with real sensors

# Swap the stock emulator HALs for ours.
PRODUCT_PACKAGES := $(filter-out @REMOVE_PACKAGES@,$(PRODUCT_PACKAGES))
PRODUCT_PACKAGES += andros_graphqld andros_sensors_subhal andros_health andros_gnss

# The stock multihal sensors service stays; only its sub-HAL list changes.
PRODUCT_COPY_FILES := $(filter-out %/hals.conf:%,$(PRODUCT_COPY_FILES))
PRODUCT_COPY_FILES += \
    vendor/andros/sensors_subhal/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

BOARD_VENDOR_SEPOLICY_DIRS += vendor/andros/sepolicy
