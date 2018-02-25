from math import log, exp

def div_pow2(x, a):
    return 2**(log(x)/log(2) - a)

def calculate_probabilities(bits):
    from math import factorial
    comb = lambda a, b: factorial(a)/factorial(b)/factorial(a-b)
    acc = [0]
    for i in xrange(bits+1):
        acc.append(acc[-1] + comb(bits, i))
    return [div_pow2(x, bits) for x in acc[1:]]

def calculate_radius(bits, threshold=0.001):
    cdf = calculate_probabilities(bits)
    for idx, p in enumerate(cdf):
        if p > threshold:
            break

    return idx
