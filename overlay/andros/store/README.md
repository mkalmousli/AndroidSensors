# store

Static library: a memory-mapped file with one slot per catalog sensor. A slot holds a timestamp and up to
6 doubles, guarded by a seqlock so readers never block the writer. `graphqld` opens it as writer;
the HALs map it read-only. Path: `/data/vendor/andros/sensors.store`.
