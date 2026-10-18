// Shared-memory sample store: one writer (graphqld), many readers (HALs).
#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "catalog_gen.h"

namespace andros {

constexpr uint32_t kMagic = 0x41445255;  // bumped when the layout changes
constexpr uint32_t kDisabled = 1;  // per-sensor flag: no writes, no events
constexpr uint32_t kDummy = 2;     // per-sensor flag: daemon synthesises values
constexpr const char* kDefaultPath = "/data/vendor/andros/sensors.store";

struct Sample {
    int64_t ts_ns = 0;
    double v[kMaxValues] = {};
};

struct Slot {
    std::atomic<uint32_t> seq;  // odd while a write is in progress (seqlock)
    int64_t ts_ns;
    double v[kMaxValues];
    std::atomic<uint32_t> flags;
    std::atomic<uint64_t> writes;
};

struct Header {
    uint32_t magic;
    uint32_t count;
    std::atomic<uint64_t> generation;  // bumped on every write
    std::atomic<uint32_t> api_off;     // 1 = GraphQL data API disabled
};

int64_t NowNs();  // CLOCK_BOOTTIME, the timebase Android sensor events use

class Store {
  public:
    // writer=true creates/initialises the file; writer=false maps it read-only.
    static Store* Open(const char* path, bool writer);
    bool Read(int idx, Sample* out) const;
    void Write(int idx, const double* values, int n);  // writer only
    uint32_t Flags(int idx) const { return slots_[idx].flags.load(std::memory_order_relaxed); }
    uint64_t Writes(int idx) const { return slots_[idx].writes.load(std::memory_order_relaxed); }
    bool ApiEnabled() const { return hdr_->api_off.load(std::memory_order_relaxed) == 0; }
    void SetFlags(int idx, uint32_t f) { if (writer_) slots_[idx].flags = f; }
    void SetApiEnabled(bool on) { if (writer_) hdr_->api_off = on ? 0 : 1; }
    uint64_t Generation() const { return hdr_->generation.load(std::memory_order_acquire); }

  private:
    Store(Header* hdr, bool writer);
    Header* hdr_;
    Slot* slots_;
    bool writer_;
};

}  // namespace andros
