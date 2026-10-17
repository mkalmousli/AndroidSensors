// Health HAL whose battery numbers come from the andros store.
#pragma once
#include <health-impl/Health.h>

#include <memory>

#include "store.h"

namespace andros {

class AndrosHealth : public ::aidl::android::hardware::health::Health {
  public:
    AndrosHealth(std::string name, std::unique_ptr<struct healthd_config>&& config)
        : Health(std::move(name), std::move(config)) {}

  protected:
    void UpdateHealthInfo(::aidl::android::hardware::health::HealthInfo* info) override;

  private:
    Store* store();
    Store* store_ = nullptr;
};

}  // namespace andros
