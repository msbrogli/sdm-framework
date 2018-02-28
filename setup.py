from distutils.core import setup, Extension
import subprocess

# Classifiers: https://pypi.python.org/pypi?%3Aaction=list_classifiers

macros = [
    ('SDM_USE_BITCOUNT_TABLE', None),
    ('SDM_ENABLE_OPENCL', None),
]
extra_compile_args = None
extra_link_args = None
libraries = None

try:
    os_string = subprocess.check_output(['uname', '-s']).strip().lower()
except OSError:
    os_string = None

if os_string == 'darwin':
    macros.append(('OS_OSX', None))
    extra_link_args = ['-framework', 'OpenCL']

#elif os_string == 'linux':
else:
    macros.append(('OS_LINUX', None))
    libraries = ['OpenCL', 'pthread', 'bsd']


libsdm_ext = Extension('sdm._libsdm', [
    'src/bitstring.c', 'src/address_space.c', 'src/counter.c',
    'src/scanner_thread.c', 'src/scanner_opencl.c', 
    'src/scanner_thread2.c', 'src/scanner_opencl2.c',
    'src/operations.c', 'src/lib/base64.c',
], libraries=libraries, define_macros=macros, extra_compile_args=extra_compile_args, extra_link_args=extra_link_args)

setup(name='sdm',
    version='1.4.0',
    license='GPLv2',
    author='Marcelo Salhab Brogliato',
    author_email='msbrogli@vialink.com.br',
    description='Sparse Distributed Memory Framework',
    long_description='',
    ext_modules=[libsdm_ext],
    packages=['sdm'],
    package_data={'sdm': ['scanner_opencl.cl', 'scanner_opencl2.cl']},
    url='https://github.com/msbrogli/sdm-framework',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Natural Language :: English',
        'Topic :: Scientific/Engineering',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX',
    ]
)
