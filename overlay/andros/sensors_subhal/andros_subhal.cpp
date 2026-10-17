#include "andros_subhal.h"

#include "sensor_table.h"

namespace andros {

Return<Result> AndrosSubHal::initialize(const sp<IHalProxyCallback>& cb) {
    callback_ = cb;
    active_.assign(HalSensors().size() + 1, false);
    poller_ = std::make_unique<Poller>([this](const std::vector<sensors::V2_1::Event>& events) {
        callback_->postEvents(events, callback_->createScopedWakelock(false));
    });
    poller_->Start();
    return Result::OK;
}

Return<Result> AndrosSubHal::setOperationMode(OperationMode) { return Result::OK; }

Return<void> AndrosSubHal::getSensorsList_2_1(getSensorsList_2_1_cb cb) {
    hidl_vec<sensors::V2_1::SensorInfo> list;
    list.resize(HalSensors().size());
    for (size_t i = 0; i < list.size(); ++i) list[i] = MakeInfo(int32_t(i + 1));
    cb(list);
    return Void();
}

Return<Result> AndrosSubHal::activate(int32_t handle, bool enabled) {
    if (HandleToIndex(handle) < 0 || !poller_) return Result::BAD_VALUE;
    active_[handle] = enabled;
    poller_->Activate(handle, enabled);
    return Result::OK;
}

Return<Result> AndrosSubHal::batch(int32_t handle, int64_t period_ns, int64_t) {
    if (HandleToIndex(handle) < 0 || !poller_) return Result::BAD_VALUE;
    poller_->SetPeriod(handle, period_ns);
    return Result::OK;
}

Return<Result> AndrosSubHal::flush(int32_t handle) {
    if (HandleToIndex(handle) < 0 || !poller_ || !active_[handle]) return Result::BAD_VALUE;
    poller_->Flush(handle);
    return Result::OK;
}

Return<Result> AndrosSubHal::injectSensorData_2_1(const sensors::V2_1::Event&) {
    return Result::INVALID_OPERATION;  // data comes from the GraphQL API, not data injection
}

Return<void> AndrosSubHal::debug(const hidl_handle&, const hidl_vec<hidl_string>&) {
    return Void();
}

Return<void> AndrosSubHal::registerDirectChannel(const SharedMemInfo&, registerDirectChannel_cb cb) {
    cb(Result::INVALID_OPERATION, -1);
    return Void();
}

Return<Result> AndrosSubHal::unregisterDirectChannel(int32_t) { return Result::INVALID_OPERATION; }

Return<void> AndrosSubHal::configDirectReport(int32_t, int32_t, RateLevel,
                                              configDirectReport_cb cb) {
    cb(Result::INVALID_OPERATION, -1);
    return Void();
}

}  // namespace andros
