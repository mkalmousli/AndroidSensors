# Hardware coverage

What the image does for each hardware area. "Bridge" = a HAL of ours reads values from the GraphQL
store, so a real phone (or you) can drive it. "Stock" = the unmodified goldfish emulator HAL.

| Area | Today | Plan |
|---|---|---|
| Motion / environment / body sensors (35 types) | Bridge | done; head tracker + heading need AIDL sensors |
| Location (GNSS) | Bridge | done; add satellites / NMEA from the phone |
| Battery / charger | Bridge | done; add per-cell health, cycle count |
| Thermal (CPU/skin temps) | Stock | Bridge: `thermal` AIDL HAL fed by the store |
| Vibrator, lights (output) | Stock | Bridge out: HAL writes events to the store; GraphQL subscription forwards them to the phone |
| Fingerprint / face (auth) | Stock (virtual) | Bridge: `triggerFingerprint` / enroll via store |
| Camera | Stock (virtual scene or host webcam) | Emulator flags map a real camera; phone-as-webcam is a host-side feed, not an image change |
| Audio in/out | Stock | Host audio; phone mic/speaker is a host-side stream |
| Telephony / SMS | Stock modem simulator | Drive with `adb emu gsm` / `sms send`; GraphQL wrapper later |
| Wi-Fi, Bluetooth, NFC | Stock (virtual Wi-Fi, BT emulation) | NFC: none in the goldfish image, needs a new HAL |
| Display / touch / keys | Stock | scrcpy over adb |

Order of work: thermal, outputs (vibrator/lights), biometrics, then connectivity wrappers.
