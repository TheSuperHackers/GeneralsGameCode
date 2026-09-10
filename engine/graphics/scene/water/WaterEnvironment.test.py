"""Verify the offline environment image consumed by the water shader.

Run: python engine/graphics/scene/water/WaterEnvironment.test.py
"""

import importlib.util
from pathlib import Path
import struct
import unittest

source = Path(__file__).resolve().parents[4] / "scripts" / "import_ra3_water.py"
spec = importlib.util.spec_from_file_location("water_import", source)
water_import = importlib.util.module_from_spec(spec)
spec.loader.exec_module(water_import)


def cube(faces):
    header = [0] * 31
    header[0], header[1], header[2], header[3] = 124, 4103, 1, 1
    header[18], header[19], header[20] = 32, 4, 113
    header[26], header[27] = 4104, 0xfe00
    return b"DDS " + struct.pack("<31I", *header) + b"".join(
        struct.pack("<4e", *rgb, 1) for rgb in faces)


def decode(image, x, y, width):
    b, g, r, multiplier = image[18 + (y * width + x) * 4:22 + (y * width + x) * 4]
    return [channel / 255 * multiplier / 255 * 4 for channel in (r, g, b)]


class WaterEnvironmentTests(unittest.TestCase):
    def test_hdr_radiance_survives_gamma_conversion_and_encoding(self):
        image = water_import.environment_rgbm(cube([(1.25, .5, .125)] * 6), 16, 8)
        for y in range(8):
            for x in range(16):
                actual = decode(image, x, y, 16)
                self.assertGreater(actual[0], 1)
                for value, expected in zip(actual, (1.25 ** 2.2, .5 ** 2.2, .125 ** 2.2)):
                    self.assertAlmostEqual(value, expected, delta=.004)

    def test_cube_axes_match_shader_longitude_and_latitude(self):
        faces = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 1, 0), (1, 0, 1), (0, 1, 1)]
        image = water_import.environment_rgbm(cube(faces), 32, 16)
        # +X, -X, +Y, -Y, +Z, -Z in Environment_UV's Z-up mapping.
        for (x, y), expected in zip(((16, 8), (0, 8), (24, 8), (8, 8), (16, 0), (16, 15)), faces):
            for value, wanted in zip(decode(image, x, y, 32), expected):
                self.assertAlmostEqual(value, wanted, delta=.004)

    def test_unsupported_layout_and_radiance_are_rejected(self):
        with self.assertRaises(ValueError):
            water_import.environment_rgbm(cube([(1, 1, 1)] * 6)[:-1], 4, 2)
        with self.assertRaises(ValueError):
            water_import.environment_rgbm(cube([(4, 4, 4)] * 6), 4, 2)


if __name__ == "__main__":
    unittest.main()
