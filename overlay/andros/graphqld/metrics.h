// Process-wide counters shown in the settings page.
#pragma once
#include <atomic>
#include <cstdint>

namespace andros {

struct Metrics {
    std::atomic<uint64_t> requests{0}, errors{0}, streams{0};
    int64_t start_ns = 0;
};

Metrics& GetMetrics();

}  // namespace andros
