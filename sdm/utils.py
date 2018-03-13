# encoding: utf-8

from __future__ import print_function
from math import log
from collections import defaultdict


def test_kernels(bits, sample, radius):
    import sdm as sdmlib
    address_space = sdmlib.AddressSpace.init_random(bits, sample)
    address_space.opencl_init()
    bs1 = sdmlib.Bitstring.init_random(bits)
    expected = set(address_space.scan_thread2(bs1, radius))
    valid_kernels = []
    for kernel in sorted(sdmlib.OPENCL_KERNEL_NAMES):
        address_space.set_opencl_kernel(kernel)
        selected = set(address_space.scan_opencl2(bs1, radius))
        if selected == expected:
            valid_kernels.append(kernel)
    return valid_kernels


def div_pow2(x, a):
    try:
        r = x
        for _ in range(a):
            r = r / 2.0
        return r
    except OverflowError:
        return 2**(log(x) / log(2) - a)


def calculate_probabilities(bits):
    from math import factorial
    comb = lambda a, b: factorial(a) // factorial(b) // factorial(a - b)
    acc = [0]
    for i in range(bits + 1):
        acc.append(acc[-1] + comb(bits, i))
    return [div_pow2(x, bits) for x in acc[1:]]


def __calculate_radius(bits, threshold=0.001):
    from math import factorial
    comb = lambda a, b: factorial(a) // factorial(b) // factorial(a - b)
    x = 0
    for i in range(bits + 1):
        x += comb(bits, i)
        p = div_pow2(x, bits)
        if p >= threshold:
            break
    return i


def calculate_radius(bits, threshold=0.001):
    from scipy.stats import norm
    return int(bits / 2.0 + norm.ppf(threshold) * (bits**0.5) / 2.0)

    if threshold == 0.001:
        # norminv(0.001) = 3.090232306167814
        return int(bits / 2.0 - 3.090232306167814 * (bits**0.5) / 2.0)
    return __calculate_radius(bits, threshold)


def calculate_probability(bits, radius):
    from scipy.stats import binom
    return binom.cdf(radius, bits, 0.5)


def genM(n, h):
    """ Generate matrix M of Geometrical treatment and statistical
    modelling of the distribution of patterns in the n-dimensional
    Boolean space by de Pádua and Aleksander (1995).
    """
    from math import factorial

    def comb(a, b):
        return factorial(a) // factorial(b) // factorial(a - b)

    M = defaultdict(int)
    for i in range(n):
        x = h + i
        y = i
        if 0 <= x < n and 0 <= y < n:
            M[(x, y)] = comb(n - h, i)

        x = h - i
        y = i
        if 0 <= x < n and 0 <= y < n:
            M[(x, y)] = comb(h, i)

    for j in range(1, h + 1):
        mult = M[(h - j, j)]
        for i in range(1, n):
            x = h + i
            y = i
            nx = x - j
            ny = y + j

            if 0 <= x < n and 0 <= y < n and 0 <= nx < n and 0 <= ny < n:
                M[(nx, ny)] = mult * M[(x, y)]

    return M


def circle_intersection(bits, sample, radius, d):
    """ Calculate the average number of hardlocations in the intersection of two circle with equal radius
    around bitstrings with distance d.
    """
    total = 0
    M = genM(bits, d)
    for i in range(radius + 1):
        for j in range(radius + 1):
            total += M[(i, j)]
    return 1.0 * sample * total / (2**bits)
