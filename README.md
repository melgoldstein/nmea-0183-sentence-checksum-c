NMEA 0183 sentence checksum
Validates GPGGA/GPRMC and GNGGA/GNRMC sentences from standard input.
Build: make; regression tests: make test (Python 3).
Run: ./nmea_check < receiver.log
Output: sentence,latitude_deg,longitude_deg,checksum (CSV).
Checks XOR bytes between $ and *, accepting upper/lower hex digits.
Converts degrees/minutes to signed decimal latitude and longitude.
Rejects unavailable fixes, bad coordinates, checksums and oversized lines.
Exit status is nonzero if any input record fails; diagnostics go to stderr.
Requires a C99 compiler and libm. License: MIT.
