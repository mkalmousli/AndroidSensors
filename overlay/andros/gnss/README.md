# gnss

`IGnss` AIDL HAL (`gnss.rc`, `gnss.xml`) replacing the emulator's. Every GPS fix is the latest
`location` sample in the store (lat, lon, alt, speed, bearing, accuracy). `gnss_extensions` holds the
required extension interfaces.
