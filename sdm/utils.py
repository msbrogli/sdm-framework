from math import log, exp

def div_pow2(x, a):
    try:
        r = x
        for _ in range(a):
            r = r/2.0
        return r
    except OverflowError:
        return 2**(log(x)/log(2) - a)

def calculate_probabilities(bits):
    from math import factorial
    comb = lambda a, b: factorial(a)/factorial(b)/factorial(a-b)
    acc = [0]
    for i in xrange(bits+1):
        acc.append(acc[-1] + comb(bits, i))
    return [div_pow2(x, bits) for x in acc[1:]]

def __calculate_radius(bits, threshold=0.001):
    from math import factorial
    comb = lambda a, b: factorial(a)/factorial(b)/factorial(a-b)
    x = 0
    for i in xrange(bits+1):
        x += comb(bits, i)
        p = div_pow2(x, bits)
        if p >= threadhold:
            break
    return i

def calculate_radius(bits, threshold=0.001):
    from scipy.stats import norm
    return int(bits/2.0 + norm.ppf(threshold)*(bits**0.5)/2.0)
    if threadhold == 0.001:
        # norminv(0.001) = 3.090232306167814
        return int(bits/2.0 - 3.090232306167814*(bits**0.5)/2.0)
    return __calculate_radius(bits, threshold)
