C API
=====


Bitstring's functions
---------------------

The following functions are used to create, destroy, and manipulate bitstrings. Many of them have a parameter `len`, which is the number of bytes of the bitstring. It was a design choice which saves computing it everytime.

.. c:type:: typedef uint64_t bitstring_t

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

.. c:function:: struct address_space_s

    .. c:member:: unsigned int bits

        SDM dimension.

    .. c:member:: unsigned int sample

        Number of hard-locations.

    .. c:member:: bitstring_t **addresses

        This approach allocates a continuous chunk of memory for all bitstring addresses.
        The `addresses` allows the use of array notation: addresses[0], addresses[1], ...

        Let `a` be `addresses`. Then::

                      a[0]   a[1]   a[2]   a[3]   a[4]
                      |      |      |      |      |
                      v      v      v      v      v
            bs_data = xxxxxx|xxxxxx|xxxxxx|xxxxxx|xxxxxx

    .. c:member:: unsigned int bs_len;

    .. c:member:: unsigned int bs_bits_remaining;

    .. c:member:: bitstring_t *bs_data;


.. c:function:: int as_init(struct address_space_s *this, unsigned int bits, unsigned int sample)

    Testing...


.. c:function:: int as_init_random(struct address_space_s *this, unsigned int bits, unsigned int sample)

    Testing again..

.. c:function:: int as_init_from_b64_file(struct address_space_s *this, char *filename)


.. c:function:: int as_free(struct address_space_s *this)

.. c:function:: int as_save_b64_file(const struct address_space_s *this, char *filename)

.. c:function:: int as_scan_linear(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *buf)

.. c:function:: int as_scan_thread(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *buf, unsigned int thread_count)

.. c:function:: void as_print_summary(struct address_space_s *this)

.. c:function:: void as_print_addresses_b64(struct address_space_s *this)

.. c:function:: void as_print_addresses_hex(struct address_space_s *this)


OpenCL Scanner
--------------

.. c:function:: int as_scanner_opencl_init(struct opencl_scanner_s *this, struct address_space_s *as, char *opencl_source)
.. c:function:: void as_scanner_opencl_free(struct opencl_scanner_s *this)
.. c:function:: int as_scan_opencl(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, uint8_t *result)


Counter's functions
-------------------
.. c:type:: typedef int counter_t

.. c:type:: struct counter_s

    .. c:member:: unsigned int bits

    .. c:member:: unsigned int sample

    .. c:member:: int fd

    .. c:member:: char *filename

    .. c:member:: counter_t **counter

    .. c:member:: counter_t *data

.. c:function:: int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample)
.. c:function:: int counter_init_file(char *filename, struct counter_s *this)
.. c:function:: void counter_free(struct counter_s *this)
.. c:function:: void counter_print_summary(struct counter_s *this)
.. c:function:: void counter_print(struct counter_s *this, unsigned int index)
.. c:function:: int counter_add_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs)
.. c:function:: int counter_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2)
.. c:function:: int counter_to_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs)
.. c:function:: int counter_create_file(char *filename, unsigned int bits, unsigned int sample)


SDM's functions
---------------

.. c:type:: struct sdm_s

    .. c:member:: unsigned int bits

    .. c:member:: unsinged int sample

    .. c:member:: unsinged int scanner_type

        .. c:macro:: SDM_SCANNER_LINEAR
        .. c:macro:: SDM_SCANNER_THREAD
        .. c:macro:: SDM_SCANNER_OPENCL

    .. c:member:: struct opencl_scanner_s *opencl_opts

    .. c:member:: unsinged int thread_count

    .. c:member:: struct address_space_s *address_space

    .. c:member:: struct counter_s *counter


.. c:function:: int sdm_init_linear(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter)
.. c:function:: int sdm_init_thread(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, unsigned int thread_count)
.. c:function:: int sdm_init_opencl(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, char *opencl_source)
.. c:function:: void sdm_free(struct sdm_s *sdm)

.. c:function:: int sdm_write(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *datum)
.. c:function:: int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output)
.. c:function:: int sdm_iter_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, unsigned int max_iter, bitstring_t *output)

