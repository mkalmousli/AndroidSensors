// Shared-memory sample store: one writer (graphqld), many readers (HALs).
#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "catalog_gen.h"

namespace andros {

constexpr uint32_t kMagic = 0x41445254;  // "ADRT" (bumped: slot size changed)
constexpr const char* kDefaultPath = "/data/vendor/andros/sensors.store";

struct Sample {
    int64_t ts_ns = 0;
    double v[kMaxValues] = {};
};

struct Slot {
    std::atomic<uint32_t> seq;  // odd while a write is in progress (seqlock)
    int64_t ts_ns;
    double v[kMaxValues];
};

struct Header {
    uint32_t magic;
    uint32_t count;
    std::atomic<uint64_t> generation;  // bumped on every write
};

int64_t NowNs();  // CLOCK_BOOTTIME, the timebase Android sensor events use

class Store {
  public:
    // writer=true creates/initialises the file; writer=false maps it read-only.
    static Store* Open(const char* path, bool writer);
    bool Read(int idx, Sample* out) const;
    void Write(int idx, const double* values, int n);  // writer only
    uint64_t Generation() const { return hdr_->generation.load(std::memory_order_acquire); }

  private:
    Store(Header* hdr, bool writer);
    Header* hdr_;
    Slot* slots_;
    bool writer_;
};

}  // namespace andros
