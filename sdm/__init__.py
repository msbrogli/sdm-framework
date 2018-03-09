
from __future__ import print_function
from builtins import range

from ctypes import cdll, cast, sizeof
from ctypes import Structure, POINTER, pointer, create_string_buffer
from ctypes import c_uint, c_uint64, c_char_p, c_int, c_void_p, c_double, c_size_t
import os
import sys

def get_lib_fullpath():
    try:
        from sysconfig import get_config_var
    except ImportError:
        def get_config_var(name):
            if name == 'SO':
                return '.so'
            raise NotImplemented

    if sys.version_info >= (3, 3):
        ext = get_config_var('EXT_SUFFIX')
    else:
        ext = get_config_var('SO')

    fullpath = os.path.join(basedir, '_libsdm'+ext)
    if not os.path.isfile(fullpath):
        fullpath = os.path.join(basedir, '_libsdm.so')

    return fullpath


bitstring_t = c_uint64
counter_t = c_int

basedir = os.path.dirname(__file__)
try:
    libsdm = cdll.LoadLibrary(get_lib_fullpath())
except Exception as e:
    if os.environ.get('GEN_DOCS'):
        libsdm = None
    else:
        raise e

if libsdm is not None:
    libsdm.bs_alloc.restype = POINTER(bitstring_t)
    libsdm.bs_init_bitcount_table()

opencl_source_code = os.path.join(basedir, 'scanner_opencl.cl').encode()
opencl2_source_code = os.path.join(basedir, 'scanner_opencl2.cl').encode()
if not os.path.exists(opencl_source_code):
    raise Exception('scanner_opencl.cl not found')
if not os.path.exists(opencl2_source_code):
    raise Exception('scanner_opencl2.cl not found')


def _multK(value, K):
    x = value // K
    if value % K != 0:
        x += 1
    x *= K
    return x

def opencl_worksize_mult16(address_space):
    local_worksize = _multK(address_space.bs_len, 16)
    mcu = address_space.opencl_opts.max_compute_units
    global_worksize = _multK(address_space.sample // 20, 2*mcu*local_worksize)
    return local_worksize, global_worksize

def opencl_worksize_power2(address_space):
    local_worksize = 1
    while local_worksize < address_space.bs_len:
        local_worksize *= 2
    mcu = address_space.opencl_opts.max_compute_units
    global_worksize = _multK(address_space.sample // 20, 2*mcu*local_worksize)
    return local_worksize, global_worksize

def opencl_worksize_sample(address_space):
    local_worksize = 0
    global_worksize = address_space.sample
    return local_worksize, global_worksize

OPENCL_KERNEL_WORKSIZE = {
    'single_scan0': opencl_worksize_sample,
    'single_scan1': opencl_worksize_mult16,
    'single_scan2': opencl_worksize_mult16,
    'single_scan3': opencl_worksize_power2,
    'single_scan4': opencl_worksize_mult16,
    'single_scan5': opencl_worksize_power2,
    'single_scan5_unroll': opencl_worksize_power2,
    'single_scan6': opencl_worksize_mult16,
}
OPENCL_KERNEL_NAMES = OPENCL_KERNEL_WORKSIZE.keys()


class AddressSpace(Structure):
    ''' The AddressSpace contains the hard-locations' addresses. Thus, it is required to
    specify the number of bits (`bits`) of each hard-location and also the number of hard-locations
    which will be generated (`sample`).

    In his book, Kanerva usually uses a 1000-bit address space with 1,000,000 hard-locations (`bits=1000` and `sample=1000000`).
    '''
    class OpenCLOptions(Structure):
        _fields_ = [
            ('kernel_name', c_char_p),
            ('global_worksize', c_size_t),
            ('local_worksize', c_size_t),
            ('max_compute_units', c_uint),
            ('verbose', c_uint),
        ]

    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('c_opencl_opts', POINTER(OpenCLOptions)),
        ('verbose', c_uint),
        ('addresses', POINTER(POINTER(bitstring_t))),
        ('bs_len', c_uint),
        ('bs_bits_remaining', c_uint),
        ('bs_data', POINTER(bitstring_t)),
    ]

    def __del__(self):
        libsdm.as_free(pointer(self))

    def __init__(self, *args, **kwargs):
        super(AddressSpace, self).__init__()
        if kwargs.get('bits', None) != 0:
            raise Exception('You must use one initializer.')

    @classmethod
    def init_random(cls, bits, sample):
        ''' Initialize an address space with hard-locations randomly chosen from {0, 1}^`bits` space.
        '''
        self = cls(bits=0)
        ret = libsdm.as_init_random(pointer(self), c_uint(bits), c_uint(sample))
        if ret != 0:
            raise Exception('Unable to create AddressSpace. Error code: {}'.format(ret))
        return self

    @classmethod
    def init_from_b64_file(cls, filename):
        ''' Load an address space from a file.
        '''
        self = cls(bits=0)
        ret = libsdm.as_init_from_b64_file(pointer(self), c_char_p(filename))
        if ret != 0:
            raise Exception('Unable to create AddressSpace. Error code: {}'.format(ret))
        return self

    def get_bitstring(self, index):
        bs = Bitstring(self.bits)
        libsdm.bs_copy(bs.bs_data, self.addresses[index], c_uint(self.bs_len))
        return bs

    @property
    def opencl_opts(self):
        return self.c_opencl_opts.contents

    def print_summary(self):
        libsdm.as_print_summary(pointer(self))

    def test_opencl_kernel(self, name):
        raise NotImplemented

    def set_opencl_kernel(self, name):
        if name not in OPENCL_KERNEL_WORKSIZE:
            raise Exception('Kernel not found.')

        if isinstance(name, str):
            self.opencl_kernel_name = name.encode()
        else:
            self.opencl_kernel_name = name

        self.opencl_opts.kernel_name = c_char_p(self.opencl_kernel_name)

        local_worksize, global_worksize = OPENCL_KERNEL_WORKSIZE[name](self)
        self.set_opencl_worksize(local_worksize, global_worksize)

    def set_opencl_worksize(self, local_worksize, global_worksize):
        self.opencl_opts.local_worksize = c_size_t(local_worksize)
        self.opencl_opts.global_worksize = c_size_t(global_worksize)

    def scan_linear(self, bs, radius):
        ''' Scan which hard-locations are in the circle with center `bs` and a given `radius`.
        The scan is performed in O(`sample`).

        It returns a list with the indexes of the hard-locations inside the circle.

        This method returns exactly the same result as :py:func:`AddressSpace.scan_thread`.
        '''
        buf = create_string_buffer(self.sample)
        libsdm.as_scan_linear(pointer(self), bs.bs_data, c_uint(radius), buf)
        return [i for i, x in enumerate(buf) if x != '\x00']

    def scan_thread(self, bs, radius, thread_count):
        ''' Scan which hard-locations are in the circle with center `bs` and a given `radius`.
        The scan is distributed among threads in O(`sample`/`thread_count`).

        It returnes a list with the indexes of the hard-locations inside the circle.

        This method returns exactly the same result as :py:func:`AddressSpace.scan_linear`.
        '''
        buf = create_string_buffer(self.sample)
        libsdm.as_scan_thread(pointer(self), bs.bs_data, c_uint(radius), buf, c_uint(thread_count))
        return [i for i, x in enumerate(buf) if x != '\x00']

    def opencl_init(self):
        return libsdm.as_scanner_opencl_init(self.c_opencl_opts, pointer(self), c_char_p(opencl2_source_code))

    def opencl_free(self):
        return libsdm.as_scanner_opencl_free(self.c_opencl_opts)

    def scan_linear2(self, bs, radius):
        # See https://docs.python.org/3/library/ctypes.html#type-conversions
        selected = (c_uint * (self.sample))()
        cnt = libsdm.as_scan_linear2(pointer(self), bs.bs_data, c_uint(radius), selected)
        return selected[:cnt]

    def scan_thread2(self, bs, radius, thread_count=4):
        # See https://docs.python.org/3/library/ctypes.html#type-conversions
        selected = (c_uint * (self.sample))()
        cnt = libsdm.as_scan_thread2(pointer(self), bs.bs_data, c_uint(radius), selected, thread_count)
        return selected[:cnt]

    def scan_opencl2(self, bs, radius):
        ''' Scan which hard-locations are in the circle with center `bs` and a given `radius`.
        The scan is distributed among threads in O(`sample`/`thread_count`).

        It returnes a list with the indexes of the hard-locations inside the circle.

        This method returns exactly the same result as :py:func:`AddressSpace.scan_linear`.
        '''
        # See https://docs.python.org/3/library/ctypes.html#type-conversions
        selected = (c_uint * (self.sample))()
        cnt = libsdm.as_scan_opencl2(self.c_opencl_opts, bs.bs_data, c_uint(radius), selected)
        return selected[:cnt]

    def save(self, filename):
        ''' Save the address space in a file. You need to provide the full filename, including the extension.
        The suggested extension is `.as`.
        '''
        return libsdm.as_save_b64_file(pointer(self), filename)


class Counter(Structure):
    ''' The Counter contains the hard-locations' counters, which is basically a list of integers.
    The counter usually is stored in a file. For the typical SDM of 1,000,000 hard-locations of 1,000 bits each,
    the counters use 3.7GB of space.
    '''
    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('fd', c_int),
        ('filename', c_char_p),
        ('counter', POINTER(POINTER(counter_t))),
        ('data', POINTER(counter_t)),
    ]

    def __del__(self):
        libsdm.counter_free(pointer(self))

    def __init__(self, *args, **kwargs):
        super(Counter, self).__init__()
        if kwargs.get('bits', None) != 0:
            raise Exception('You must use one initializer.')

    @classmethod
    def init_zero(cls, bits, sample):
        ''' Initialize the counters with initial value zero. The counters are stored in RAM memory.
        '''
        self = cls(bits=0)
        ret = libsdm.counter_init(pointer(self), c_uint(bits), c_uint(sample))
        if ret != 0:
            raise Exception('Unable to create Counter. Error code: {}'.format(ret))
        return self

    @classmethod
    def load_file(cls, filename):
        ''' Load the counters from a file. It is the suggested way to use counters.
        '''
        self = cls(bits=0)
        ret = libsdm.counter_init_file(c_char_p(filename), pointer(self))
        if ret != 0:
            raise Exception('Unable to create Counter. Error code: {}'.format(ret))
        return self

    @classmethod
    def create_file(cls, filename, bits, sample):
        ''' Create a new file to store counters. You do not need to provide any file extension,
        because the counters are store in two files and the extensions will be automatically added.
        '''
        ret = libsdm.counter_create_file(c_char_p(filename), c_uint(bits), c_uint(sample))
        if ret != 0:
            raise Exception('Unable to create Counter. Error code: {}'.format(ret))
        return cls.load_file(filename)

    def print_summary(self):
        ''' Print a summary of the counters.
        '''
        libsdm.counter_print_summary(pointer(self))

    def print_address(self, index):
        ''' Print the counters of a hard-location. The index must match the index in the associated AddressSpace.
        '''
        libsdm.counter_print(pointer(self), c_uint(index))

    def add_bitstring(self, bs):
        ''' Add a bitstring to the counter of a hard-location.
        '''
        libsdm.counter_add_bitstring(pointer(self), c_uint(index), bs.bs_data)

    def add_counter(self, idx1, counter, idx2):
        libsdm.counter_add_counter(pointer(self), c_uint(idx1), pointer(counter), c_uint(idx2))

    def to_bitstring(self, index):
        ''' Returns a bitstring associated to the counters of the hard-location.
        '''
        bs = Bitstring(self.bits)
        libsdm.counter_to_bitstring(pointer(self), c_uint(index), bs.bs_data)
        return bs

    def save(self, filename):
        libsdm.counter_save_file(pointer(self), c_char_p(filename))


class Bitstring(object):
    ''' The Bitstring is the basic unity of storage in an SDM.
    '''
    def __init__(self, bits):
        '''  Initialize a new bitstring. The value of the bitstring is not initialized.
        '''
        self.bits = bits
        self.bs_len = bits // 8 // sizeof(bitstring_t)
        if (self.bs_len * 8 * sizeof(bitstring_t) < bits):
            self.bs_len += 1
        self.bs_remaining_bits = self.bs_len * 8 * sizeof(bitstring_t) - self.bits
        self.bs_data = libsdm.bs_alloc(c_uint(self.bs_len))

    def __del__(self):
        libsdm.bs_free(self.bs_data)

    @classmethod
    def init_hex(cls, bits, hex_str):
        ''' Initialize a bitstring based on an hexadecimal string.
        '''
        self = cls(bits)
        libsdm.bs_init_hex(self.bs_data, c_uint(self.bs_len), hex_str)
        return self

    @classmethod
    def init_b64(cls, b64):
        ''' Initialize a bitstring based on a base-64 string.
        '''
        self = cls(bits)
        libsdm.bs_init_b64(self.bs_data, b64)
        return self

    @classmethod
    def init_random(cls, bits):
        ''' Initialize a random bitstring.
        '''
        self = cls(bits)
        libsdm.bs_init_random(self.bs_data, c_uint(self.bs_len), c_uint(self.bs_remaining_bits))
        return self

    @classmethod
    def init_ones(cls, bits):
        ''' Initialize a bitstring with all bits equal to one.
        '''
        self = cls(bits)
        libsdm.bs_init_ones(self.bs_data, c_uint(self.bs_len), c_uint(self.bs_remaining_bits))
        return self

    @classmethod
    def init_zeros(cls, bits):
        ''' Initialize a bitstring with all bits equal to one.
        '''
        self = cls(bits)
        libsdm.bs_init_zeros(self.bs_data, c_uint(self.bs_len), c_uint(self.bs_remaining_bits))
        return self

    @classmethod
    def init_from_bitstring(cls, other):
        ''' Initialize a bitstring copying the bits from `other` bitstring.
        '''
        self = cls(other.bits)
        libsdm.bs_copy(self.bs_data, other.bs_data, c_uint(self.bs_len))
        return self

    def __hash__(self):
        return hash(self.to_hex())

    def __xor__(self, other):
        bs = Bitstring.init_from_bitstring(self)
        bs.xor(other)
        return bs

    def copy(self):
        return Bitstring.init_from_bitstring(self)

    def get_bit(self, bit):
        ''' Return the value of a specific bit.
        '''
        return libsdm.bs_get_bit(self.bs_data, c_uint(bit))

    def set_bit(self, bit, value):
        ''' Change a specific bit.
        '''
        return libsdm.bs_set_bit(self.bs_data, c_uint(bit), c_uint(value))

    def flip_bit(self, bit, value):
        ''' Flip a specific bit.
        '''
        return libsdm.bs_flip_bit(self.bs_data, c_uint(bit))

    def flip_random_bits(self, flips):
        ''' Randomly flip some bits in the bitstring.

        It may be used to generate a bitstring with a given distance from another one.
        '''
        return libsdm.bs_flip_random_bits(self.bs_data, c_uint(self.bits), c_uint(flips))

    def to_b64(self):
        ''' Return a base-64 string of the bitstring.
        '''
        buf = create_string_buffer(self.bits)
        libsdm.bs_to_b64(buf, self.bs_data, c_uint(self.bs_len))
        return buf.value

    def to_hex(self):
        ''' Return an hexadecimal string of the bitstring.
        '''
        buf = create_string_buffer(self.bits)
        libsdm.bs_to_hex(buf, self.bs_data, c_uint(self.bs_len))
        return buf.value

    def to_binary(self):
        return ''.join([str(self.get_bit(i)) for i in range(self.bits)])

    def distance_to(self, other):
        ''' Return the hamming distance to `other` bitstring.
        '''
        if self.bits != other.bits:
            raise Exception('Dimensions must be equal.')
        return libsdm.bs_distance(self.bs_data, other.bs_data, c_uint(self.bs_len))

    def xor(self, other):
        ''' Calculates the XOR between bitstrings `self` and `other`.
        The result is stored in `self`.
        '''
        if self.bits != other.bits:
            raise Exception('Dimensions must be equal.')
        return libsdm.bs_xor(self.bs_data, other.bs_data, c_uint(self.bs_len))

    def op_and(self, other):
        ''' Calculates the OR between bitstrings `self` and `other`.
        The result is stored in `self`.
        '''
        if self.bits != other.bits:
            raise Exception('Dimensions must be equal.')
        return libsdm.bs_and(self.bs_data, other.bs_data, c_uint(self.bs_len))

    def op_or(self, other):
        ''' Calculates the AND between bitstrings `self` and `other`.
        The result is stored in `self`.
        '''
        if self.bits != other.bits:
            raise Exception('Dimensions must be equal.')
        return libsdm.bs_or(self.bs_data, other.bs_data, c_uint(self.bs_len))

    def __eq__(self, other):
        return self.distance_to(other) == 0

SDM_SCANNER_LINEAR = 1
SDM_SCANNER_THREAD = 2
SDM_SCANNER_OPENCL = 3

class SDM(Structure):
    ''' An SDM is a facade to manage a given AddressSpace and Counters.
    '''
    _fields_ = [
        ('bits', c_uint),
        ('sample', c_uint),
        ('scanner_type', c_uint),
        ('thread_count', c_uint),
        ('c_address_space_p', POINTER(AddressSpace)),
        ('c_counter_p', POINTER(Counter)),
    ]

    def __init__(self, address_space, counter, radius, scanner_type, thread_count=8):
        ''' Create a new SDM using a given address space and counters.
        The `radius` is the default radius for read and write operations.
        The `scanner_type` is the default scanner for read and write operations.
        The `thread_count` will be used only when `scanner_type` is `SDM_SCANNER_THREAD`.
        '''
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
            libsdm.sdm_init_opencl(pointer(self), pointer(address_space), pointer(counter), c_char_p(opencl2_source_code))

        self.scanner_type = scanner_type
        self.address_space = address_space
        self.counter = counter
        self.radius = radius

    def reset_hardlocation(self, index):
        return libsdm.sdm_reset_hardlocation(pointer(self), c_uint(index))

    def iter_read(self, addr, radius=None, max_iter=6):
        ''' Return an iterative reading with a maximum number of iterations.
        '''
        if radius is None:
            radius = self.radius
        out = Bitstring(self.bits)
        libsdm.sdm_iter_read2(pointer(self), addr.bs_data, c_uint(radius), c_uint(max_iter), out.bs_data)
        return out

    def read(self, addr, radius=None, z=None):
        ''' Return a single read from the SDM.
        '''
        if radius is None:
            radius = self.radius
        out = Bitstring(self.bits)
        if z is None:
            libsdm.sdm_read2(pointer(self), addr.bs_data, c_uint(radius), out.bs_data)
        else:
            libsdm.sdm_generic_read(pointer(self), addr.bs_data, c_uint(radius), out.bs_data, c_double(z))
        return out

    def read_counter(self, addr, radius=None, z=None):
        ''' Return a single read from the SDM. But, instead of returning the Bitstring, it returns the counter.
        '''
        if radius is None:
            radius = self.radius
        counter = Counter.init_zero(self.bits, 1)
        libsdm.sdm_read_counter(pointer(self), addr.bs_data, c_uint(radius), pointer(counter))
        return counter

    def write(self, addr, datum, radius=None, weight=1):
        ''' Write a bitstring to the SDM.
        '''
        if radius is None:
            radius = self.radius
        if weight == 1:
            libsdm.sdm_write2(pointer(self), addr.bs_data, c_uint(radius), datum.bs_data)
        else:
            if isinstance(weight, int):
                libsdm.sdm_write2_weighted(pointer(self), addr.bs_data, c_uint(radius), datum.bs_data, c_int(weight))
            elif isinstance(weight, (list, tuple)):
                assert(self.bits+1 == len(weight))
                # See https://docs.python.org/3/library/ctypes.html#type-conversions
                weight_buf = (c_int * (self.bits+1))(*weight)
                libsdm.sdm_write2_weighted_table(pointer(self), addr.bs_data, c_uint(radius), datum.bs_data, weight_buf)
            else:
                raise NotImplemented

    #def write_sub(self, addr, datum, radius=None):
    #    ''' Write a bitstring to the SDM.
    #    '''
    #    if radius is None:
    #        radius = self.radius
    #    libsdm.sdm_write_sub(pointer(self), addr.bs_data, c_uint(radius), datum.bs_data)

    def write_random_bitstrings(self, n):
        for _ in range(n):
            bs = Bitstring.init_random(self.bits)
            self.write(bs, bs)


def gen_sdm(scanner_type):
    bits = 1000
    sample = 1000000
    address_space = AddressSpace.init_random(bits, sample)
    counter = Counter.init_zero(bits, sample)
    sdm = SDM(address_space, counter, 451, scanner_type)
    return sdm

def test_read_write(sdm):
    bs1 = Bitstring(bits)
    print(bs1.to_b64())
    sdm.write(bs1, bs1)
    bs2 = sdm.read(bs1)
    print(bs2.to_b64())


def gen_all():
    sdm_linear = gen_sdm(SDM_SCANNER_LINEAR)
    sdm_thread = gen_sdm(SDM_SCANNER_THREAD)
    sdm_opencl = gen_sdm(SDM_SCANNER_OPENCL)
    return sdm_linear, sdm_thread, sdm_opencl

