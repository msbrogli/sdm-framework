C API
=====


Bitstring's functions
---------------------

The following functions are used to create, destroy, and manipulate bitstrings. Many of them have a parameter `len`, which is the number of bytes of the bitstring. It was a design choice which saves computing it everytime.

.. c:function:: void bs_init_bitcount_table

    Initialize a 64kb in RAM which is used to improve performance when calculating the distance between two bitstrings.


.. c:function:: bitstring_t* bs_alloc(const unsigned int len)

    Allocate memory for a bitstring with `len` bytes.


.. c:function:: void bs_free(bitstring_t *bs)

    Free the memory of a bitstring.


.. c:function:: void bs_copy(bitstring_t *dst, const bitstring_t *src, unsigned int len)

    Copy one bitstring into another.


.. c:function:: void bs_init_ones(bitstring_t *bs, unsigned int len, unsigned int bits_remaining)

    Initialize a bitstring with all bits equal to one. The bitstring's memory must have already been allocated.


.. c:function:: void bs_init_random(bitstring_t *bs, unsigned int len, unsigned int bits_remaining)

    Initialize a bitstring with random bits. Each bit is sampled from Bernoulli trial with p=0.5. The bitstring's memory must have already been allocated.


.. c:function:: void bs_init_hex(bitstring_t *bs, unsigned int len, char *hex)

    Initialize a bitstring with random bits. Each bit is sampled from Bernoulli trial with p=0.5. The bitstring's memory must have already been allocated.


.. c:function:: void bs_init_b64(bitstring_t *bs, char *b64)

    Initialize a bitstring from a base64 string. The bitstring's memory must have already been allocated.


.. c:function:: void bs_to_hex(char *buf, bitstring_t *bs, unsigned int len)

    Initialize a bitstring from a hexadecimal string. The bitstring's memory must have already been allocated.


.. c:function:: void bs_to_b64(char *buf, bitstring_t *bs, unsigned int len)

    Generate the base64 string representation of the bitstring.


.. c:function:: int bs_distance(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len)

    Calculate the hamming distance between two bitstrings.


.. c:function:: unsigned int bs_get_bit(bitstring_t *bs, unsigned int bit)

    Return a specific bit from a bitstring.


.. c:function:: void bs_set_bit(bitstring_t *bs, unsigned int bit, unsigned int value)

    Change the value of a specific bit from a bitstring.


.. c:function:: void bs_flip_bit(bitstring_t *bs, unsigned int bit)

    Flip a specific bit from a bitstring.


.. c:function:: int bs_flip_random_bits(bitstring_t *bs, unsigned int bits, unsigned int flips)

    Randomly choose `flips` bits of the bitstring. It is used to generate a random bitstring with a given distance from another bitstring.


Address Space's functions
-------------------------

.. c:function:: int as_init(struct address_space_s *this, unsigned int bits, unsigned int sample)

    Testing...


.. c:function:: int as_init_random(struct address_space_s *this, unsigned int bits, unsigned int sample)

    Testing again..


.. c:function:: int as_free(struct address_space_s *this)


