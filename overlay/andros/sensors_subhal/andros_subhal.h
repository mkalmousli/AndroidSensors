// Multihal (V2_1) sub-HAL: sensors as seen by SensorService, fed by the store.
#pragma once
#include <V2_1/SubHal.h>

#include <memory>

#include "poller.h"

namespace andros {

namespace sensors = ::android::hardware::sensors;
using ::android::sp;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using sensors::V1_0::OperationMode;
using sensors::V1_0::RateLevel;
using sensors::V1_0::Result;
using sensors::V1_0::SharedMemInfo;
using sensors::V2_1::implementation::IHalProxyCallback;
using sensors::V2_1::implementation::ISensorsSubHal;

class AndrosSubHal : public ISensorsSubHal {
  public:
    const std::string getName() override { return "andros"; }
    Return<Result> initialize(const sp<IHalProxyCallback>& cb) override;
    Return<Result> setOperationMode(OperationMode mode) override;

    Return<void> getSensorsList_2_1(getSensorsList_2_1_cb cb) override;
    Return<Result> activate(int32_t handle, bool enabled) override;
    Return<Result> batch(int32_t handle, int64_t period_ns, int64_t max_latency_ns) override;
    Return<Result> flush(int32_t handle) override;
    Return<Result> injectSensorData_2_1(const sensors::V2_1::Event& event) override;
    Return<void> debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;

    // direct-channel entry points are not supported by this sub-HAL.
    Return<void> registerDirectChannel(const SharedMemInfo&, registerDirectChannel_cb cb) override;
    Return<Result> unregisterDirectChannel(int32_t) override;
    Return<void> configDirectReport(int32_t, int32_t, RateLevel, configDirectReport_cb cb) override;

  private:
    sp<IHalProxyCallback> callback_;
    std::unique_ptr<Poller> poller_;
    std::vector<bool> active_;
};

}  // namespace andros
