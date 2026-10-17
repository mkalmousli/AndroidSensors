#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "andros_gnss.h"

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(2);
    auto gnss = ndk::SharedRefBase::make<andros::AndrosGnss>();
    const std::string name = std::string(andros::AndrosGnss::descriptor) + "/default";
    binder_status_t status = AServiceManager_addService(gnss->asBinder().get(), name.c_str());
    CHECK_EQ(status, STATUS_OK) << "failed to register " << name;
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // unreachable
}
