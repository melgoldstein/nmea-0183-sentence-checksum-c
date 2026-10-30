import pathlib
import subprocess
import tempfile
import unittest
import os

ROOT = pathlib.Path(__file__).resolve().parents[1]
SUFFIX = ".exe" if os.name == "nt" else ""
EXECUTABLE = ROOT / ("nmea_check"+SUFFIX)
if not EXECUTABLE.exists():
    EXECUTABLE = ROOT / "build" / ("nmea_check"+SUFFIX)

def invoke(*arguments, text=None):
    return subprocess.run([str(EXECUTABLE), *map(str,arguments)], input=text, text=True, capture_output=True)

class CliTests(unittest.TestCase):
    def test_help(self):
        result = invoke("--help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("usage:", result.stdout)

    def test_valid_coordinates(self):
        result = invoke(text="$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("48.1173000,11.5166667", result.stdout)

    def test_bad_checksum_and_recovery(self):
        valid = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\n"
        result = invoke(text="$bad*zz\n"+valid)
        self.assertEqual(result.returncode, 1)
        self.assertIn("line 1", result.stderr)
        self.assertIn("48.1173000", result.stdout)

    def test_oversized_line(self):
        self.assertNotEqual(invoke(text="$"+"X"*400+"\n").returncode, 0)

if __name__ == "__main__":
    unittest.main()
