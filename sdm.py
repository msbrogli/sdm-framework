
from ctypes import cdll, cast, sizeof
from ctypes import Structure, POINTER, pointer, create_string_buffer
from ctypes import c_uint, c_uint64, c_char_p, c_int, c_void_p

bitstring_t = c_uint64
counter_t = c_int

libsdm = cdll.LoadLibrary('src/libsdm.so')
libsdm.bs_alloc.restype = POINTER(bitstring_t)
libsdm.bs_init_bitcount_table()

class AddressSpace(Structure):
    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('addresses', POINTER(POINTER(bitstring_t))),
        ('bs_len', c_uint),
        ('bs_bits_remaining', c_uint),
        ('bs_data', POINTER(bitstring_t)),
    ]

    @classmethod
    def init_random(cls, bits, sample):
        self = cls()
        libsdm.as_init_random(pointer(self), c_uint(bits), c_uint(sample))
        return self

    @classmethod
    def init_from_b64_file(cls, filename):
        self = cls()
        libsdm.as_init_b64_file(pointer(self), c_char_p(filename))
        return self

    def print_summary(self):
        libsdm.as_print_summary(pointer(self))

    def scan_linear(self, bs, radius):
        buf = create_string_buffer(self.bits)
        libsdm.as_scan_linear(pointer(self), bs, c_uint(radius), buf)
        return buf


class Counter(Structure):
    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('fd', c_int),
        ('filename', c_char_p),
        ('counter', POINTER(POINTER(counter_t))),
        ('data', POINTER(counter_t)),
    ]

    @classmethod
    def init_zero(cls, bits, sample):
        self = cls()
        libsdm.counter_init(pointer(self), c_uint(bits), c_uint(sample))
        return self

    @classmethod
    def load_file(cls, filename):
        self = cls()
        libsdm.counter_init_file(c_char_p(filename), pointer(self))
        return self

    @classmethod
    def create_file(cls, filename, bits, sample):
        libsdm.counter_create_file(c_char_p(filename), c_uint(bits), c_uint(sample))
        return cls.load_file(filename)

    def print_summary(self):
        libsdm.counter_print_summary(pointer(self))

    def print_address(self, index):
        libsdm.counter_print(pointer(self), c_uint(index))

    def add_bitstring(self, bs):
        libsdm.counter_add_bitstring(pointer(self), c_uint(index), bs.bs_data)

    def add_counter(self, idx1, counter, idx2):
        libsdm.counter_add_counter(pointer(self), c_uint(idx1), pointer(counter), c_uint(idx2))

    def to_bitstring(self, index):
        bs = Bitstring(self.bits)
        libsdm.counter_to_bitstring(pointer(self), c_uint(index), bs.bs_data)
        return bs


class Bitstring(object):
    def __init__(self, bits):
        self.bits = bits
        self.bs_len = bits // 8 // sizeof(bitstring_t)
        if (self.bs_len * 8 * sizeof(bitstring_t) < bits):
            self.bs_len += 1
        self.bs_remaining_bits = self.bs_len * 8 * sizeof(bitstring_t) - self.bits
        self.bs_data = libsdm.bs_alloc(c_uint(self.bs_len))

    @classmethod
    def init_random(cls, bits):
        self = cls(bits)
        libsdm.bs_init_random(self.bs_data, c_uint(self.bs_len), c_uint(self.bs_remaining_bits))
        return self

    @classmethod
    def init_ones(cls, bits):
        self = cls(bits)
        libsdm.bs_init_ones(self.bs_data, c_uint(self.bs_len), c_uint(self.bs_remaining_bits))
        return self

    @classmethod
    def init_from_bitstring(cls, other):
        self = cls(bits)
        libsdm.bs_copy(self.bs_data, other.bs_data, c_uint(self.bs_len))
        return self

    def get_bit(self, bit):
        return libsdm.bs_get_bit(self.bs_data, c_uint(bit))

    def set_bit(self, bit, value):
        return libsdm.bs_set_bit(self.bs_data, c_uint(bit), c_uint(value))

    def flip_bit(self, bit, value):
        return libsdm.bs_flip_bit(self.bs_data, c_uint(bit))

    def flip_random_bits(self, flips):
        return libsdm.bs_flip_random_bits(self.bs_data, c_uint(self.bits), c_uint(flips))

    def to_b64(self):
        buf = create_string_buffer(1000)
        libsdm.bs_to_b64(buf, self.bs_data, c_uint(self.bs_len))
        return buf.value

    def to_hex(self):
        buf = create_string_buffer(1000)
        libsdm.bs_to_hex(buf, self.bs_data, c_uint(self.bs_len))
        return buf.value

    def distance_to(self, other):
        if self.bits != other.bits:
            raise Exception('Dimensions must be equal.')
        return libsdm.bs_distance(self.bs_data, other.bs_data, c_uint(self.bs_len))

    def __eq__(self, other):
        return self.distance_to(other) == 0

SDM_SCANNER_LINEAR = 1
SDM_SCANNER_THREAD = 2
SDM_SCANNER_OPENCL = 3

class SDM(Structure):
    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('scanner_type', c_uint),
        ('opencl_opts', POINTER(c_void_p)),
        ('thread_count', c_uint),
        ('c_address_space_p', POINTER(AddressSpace)),
        ('c_counter_p', POINTER(Counter)),
    ]

    def __init__(self, address_space, counter, radius, scanner_type, thread_count=8):
        if address_space.bits != counter.bits:
            raise Exception('Dimensions must be equal.')
        if address_space.sample != counter.sample:
            raise Exception('Sample must be equal.')
        self.c_address_space_p = pointer(address_space)
        self.c_counter_p = pointer(counter)

        if scanner_type == SDM_SCANNER_LINEAR:
            libsdm.sdm_init_linear(pointer(self), pointer(address_space), pointer(counter))
        elif scanner_type == SDM_SCANNER_THREAD:
            libsdm.sdm_init_thread(pointer(self), pointer(address_space), pointer(counter), thread_count)
        elif scanner_type == SDM_SCANNER_OPENCL:
            libsdm.sdm_init_opencl(pointer(self), pointer(address_space), pointer(counter))

        self.address_space = address_space
        self.counter = counter
        self.radius = radius

    def iter_read(self, addr, radius=None, max_iter=6):
        if radius is None:
            radius = self.radius
        out = Bitstring(self.bits)
        libsdm.sdm_iter_read(pointer(self), addr.bs_data, c_uint(radius), c_uint(max_iter), out.bs_data)
        return out

    def read(self, addr, radius=None):
        if radius is None:
            radius = self.radius
        out = Bitstring(self.bits)
        libsdm.sdm_read(pointer(self), addr.bs_data, c_uint(radius), out.bs_data)
        return out

    def write(self, addr, datum, radius=None):
        if radius is None:
            radius = self.radius
        libsdm.sdm_write(pointer(self), addr.bs_data, c_uint(radius), datum.bs_data)


def gen_sdm(scanner_type):
    bits = 1000
    sample = 1000000
    address_space = AddressSpace.init_random(bits, sample)
    counter = Counter.init_zero(bits, sample)
    sdm = SDM(address_space, counter, 451, scanner_type)
    return sdm

def test_read_write(sdm):
    bs1 = Bitstring(bits)
    print bs1.to_b64()
    sdm.write(bs1, bs1)
    bs2 = sdm.read(bs1)
    print bs2.to_b64()


def gen_all():
    sdm_linear = gen_sdm(SDM_SCANNER_LINEAR)
    sdm_thread = gen_sdm(SDM_SCANNER_THREAD)
    sdm_opencl = gen_sdm(SDM_SCANNER_OPENCL)
    return sdm_linear, sdm_thread, sdm_opencl

