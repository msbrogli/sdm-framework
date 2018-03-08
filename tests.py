import unittest
from sdm import Bitstring, AddressSpace, Counter, SDM
import sdm as sdmlib

class SDMBasicTests(unittest.TestCase):

    def test_bitstring(self):
        bs = Bitstring.init_random(1000)

    def test_bitstring_copy(self):
        bs1 = Bitstring.init_random(1000)
        bs2 = bs1.copy()
        self.assertEqual(0, bs1.distance_to(bs2))

    def test_address_space(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        bs = Bitstring.init_random(1000)
        as1.scan_thread2(bs, 451)

    def test_counter(self):
        counter = Counter.init_zero(1000, 1000000)

    def test_sdm_linear(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        counter = Counter.init_zero(1000, 1000000)
        sdm = SDM(as1, counter, 451, sdmlib.SDM_SCANNER_LINEAR)

        bs1 = Bitstring.init_random(1000)
        sdm.write(bs1, bs1)

        bs2 = sdm.read(bs1)
        self.assertEqual(0, bs1.distance_to(bs2))

    def test_sdm_thread(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        counter = Counter.init_zero(1000, 1000000)
        sdm = SDM(as1, counter, 451, sdmlib.SDM_SCANNER_THREAD)

        bs1 = Bitstring.init_random(1000)
        sdm.write(bs1, bs1)

        bs2 = sdm.read(bs1)
        self.assertEqual(0, bs1.distance_to(bs2))

    def test_sdm_opencl(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        counter = Counter.init_zero(1000, 1000000)
        sdm = SDM(as1, counter, 451, sdmlib.SDM_SCANNER_OPENCL)

        bs1 = Bitstring.init_random(1000)
        sdm.write(bs1, bs1)

        bs2 = sdm.read(bs1)
        self.assertEqual(0, bs1.distance_to(bs2))


if __name__ == '__main__':
    unittest.main()
