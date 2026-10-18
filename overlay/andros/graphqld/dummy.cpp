// Dummy mode: sensors flagged kDummy get slowly varying synthetic values around their defaults.
#include <chrono>
#include <cmath>
#include <thread>

#include "dummy.h"
#include "resolvers.h"

namespace andros {

void StartDummy(Store* store) {
    std::thread([store] {
        for (double t = 0;; t += 0.1) {
            for (int i = 0; i < kSensorCount; ++i) {
                const SensorDef& d = kSensors[i];
                if (!(store->Flags(i) & kDummy) || d.mode == Mode::kOneShot || d.mode == Mode::kSpecial)
                    continue;
                double v[kMaxValues];
                for (int k = 0; k < d.n; ++k) {
                    const double amp = d.hal == Hal::kGnss ? (k < 2 ? 2e-5 : 0.0)
                                                           : 0.05 * std::fmax(std::fabs(d.init[k]), 1.0);
                    v[k] = d.init[k] + amp * std::sin(t * (0.5 + 0.3 * k) + i);
                }
                store->Write(i, v, d.n);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

}  // namespace andros
