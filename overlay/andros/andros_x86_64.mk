# Emulator product with sensors, location and battery served by our own HALs.
# Our sensors hals.conf must come BEFORE the inherit: for a duplicate copy destination the first
# entry wins. Stock HALs are replaced by `overrides:` in our modules' Android.bp.
PRODUCT_COPY_FILES += \
    vendor/andros/sensors_subhal/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

$(call inherit-product, device/generic/goldfish/64bitonly/product/sdk_phone64_x86_64.mk)
include vendor/andros/gen/andros_features.mk

PRODUCT_NAME := andros_x86_64
PRODUCT_MODEL := Android device with real sensors

PRODUCT_PACKAGES += andros_graphqld andros_sensors_subhal andros_health andros_gnss

BOARD_VENDOR_SEPOLICY_DIRS += vendor/andros/sepolicy
