#include "store.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <ctime>

namespace andros {

namespace {
constexpr size_t kSize = sizeof(Header) + sizeof(Slot) * kSensorCount;
}

int64_t NowNs() {
    timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

Store::Store(Header* hdr, bool writer)
    : hdr_(hdr), slots_(reinterpret_cast<Slot*>(hdr + 1)), writer_(writer) {}

Store* Store::Open(const char* path, bool writer) {
    int fd = open(path, writer ? (O_RDWR | O_CREAT | O_CLOEXEC) : (O_RDONLY | O_CLOEXEC), 0664);
    if (fd < 0) return nullptr;
    if (writer && ftruncate(fd, kSize) != 0) { close(fd); return nullptr; }
    void* map = mmap(nullptr, kSize, writer ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return nullptr;
    auto* store = new Store(static_cast<Header*>(map), writer);
    if (writer && store->hdr_->magic != kMagic) {
        for (int i = 0; i < kSensorCount; ++i) store->Write(i, kSensors[i].init, kSensors[i].n);
        store->hdr_->count = kSensorCount;
        store->hdr_->magic = kMagic;
    }
    if (!writer && (store->hdr_->magic != kMagic || store->hdr_->count != kSensorCount)) {
        delete store;
        return nullptr;
    }
    return store;
}

bool Store::Read(int idx, Sample* out) const {
    if (idx < 0 || idx >= kSensorCount) return false;
    const Slot& s = slots_[idx];
    for (int tries = 0; tries < 1000; ++tries) {
        uint32_t before = s.seq.load(std::memory_order_acquire);
        if (before & 1) continue;
        out->ts_ns = s.ts_ns;
        for (int i = 0; i < 6; ++i) out->v[i] = s.v[i];
        std::atomic_thread_fence(std::memory_order_acquire);
        if (before == s.seq.load(std::memory_order_relaxed)) return true;
    }
    return false;
}

void Store::Write(int idx, const double* values, int n) {
    if (!writer_ || idx < 0 || idx >= kSensorCount) return;
    Slot& s = slots_[idx];
    uint32_t seq = s.seq.load(std::memory_order_relaxed);
    s.seq.store(seq + 1, std::memory_order_release);
    std::atomic_thread_fence(std::memory_order_release);
    s.ts_ns = NowNs();
    for (int i = 0; i < n && i < 6; ++i) s.v[i] = values[i];
    s.seq.store(seq + 2, std::memory_order_release);
    hdr_->generation.fetch_add(1, std::memory_order_release);
}

}  // namespace andros
