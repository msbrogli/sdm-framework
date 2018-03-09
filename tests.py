import unittest
from sdm import Bitstring, AddressSpace, Counter, SDM
import sdm as sdmlib
from sdm import utils

class BitstringTests(unittest.TestCase):
    def test_bitstring_init_random(self):
        bs1 = Bitstring.init_random(1000)
        cnt1 = bs1.to_binary().count('1')
        self.assertLessEqual(abs(cnt1 - 500), 100)

    def test_bitstring_init_zeros(self):
        bs1 = Bitstring.init_zeros(1000)
        cnt0 = bs1.to_binary().count('0')
        self.assertEqual(1000, cnt0)

    def test_bitstring_init_ones(self):
        bs1 = Bitstring.init_ones(1000)
        cnt1 = bs1.to_binary().count('1')
        self.assertEqual(1000, cnt1)

    def test_bitstring_copy(self):
        bs1 = Bitstring.init_random(1000)
        bs2 = bs1.copy()
        self.assertEqual(0, bs1.distance_to(bs2))

    def test_bitstring_flip_random_bits(self):
        bs1 = Bitstring.init_random(1000)
        for i in range(1001):
            bs2 = bs1.copy()
            bs2.flip_random_bits(i)
            self.assertEqual(i, bs1.distance_to(bs2))

    def test_bitstring_xor(self):
        bs1 = Bitstring.init_random(1000)

        bs2 = Bitstring.init_ones(1000)
        bs3 = bs1.copy()
        bs3.xor(bs2)
        self.assertEqual(1000, bs1.distance_to(bs3))

        bs2 = Bitstring.init_zeros(1000)
        bs3 = bs1.copy()
        bs3.xor(bs2)
        self.assertEqual(0, bs1.distance_to(bs3))


class AddressSpaceTests(unittest.TestCase):
    def test_init_opencl(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        as1.opencl_init()


class SDMTests(unittest.TestCase):
    def test_radius_calculation(self):
        self.assertEqual(451, utils.calculate_radius(1000))
        self.assertEqual(103, utils.calculate_radius(256))
        self.assertEqual(4845, utils.calculate_radius(10000))

    def test_address_space(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        bs = Bitstring.init_random(1000)
        as1.scan_thread2(bs, 451)

    def test_counter(self):
        counter = Counter.init_zero(1000, 1000000)

    def _test_sdm(self, scanner_type):
        as1 = AddressSpace.init_random(1000, 1000000)
        counter = Counter.init_zero(1000, 1000000)
        sdm = SDM(as1, counter, 451, scanner_type)

        bs1 = Bitstring.init_random(1000)
        sdm.write(bs1, bs1)
        bs2 = sdm.read(bs1)
        self.assertEqual(0, bs1.distance_to(bs2))

    def test_sdm_linear(self):
        self._test_sdm(sdmlib.SDM_SCANNER_LINEAR)

    def test_sdm_thread(self):
        self._test_sdm(sdmlib.SDM_SCANNER_THREAD)

    def test_sdm_opencl(self):
        self._test_sdm(sdmlib.SDM_SCANNER_OPENCL)

    def test_sdm_opencl_kernels(self):
        as1 = AddressSpace.init_random(1000, 1000000)
        counter = Counter.init_zero(1000, 1000000)
        sdm = SDM(as1, counter, 451, sdmlib.SDM_SCANNER_OPENCL)
        as1.opencl_opts.verbose = 1
        for kernel in sdmlib.OPENCL_KERNEL_NAMES:
            as1.set_opencl_kernel(kernel)

            bs1 = Bitstring.init_random(1000)
            sdm.write(bs1, bs1)
            bs2 = sdm.read(bs1)
            self.assertEqual(0, bs1.distance_to(bs2))


if __name__ == '__main__':
    unittest.main()
