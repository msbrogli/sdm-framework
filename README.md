Sparse Distributed Memory Framework
===================================
This project intends to be a framework which can be adapted to any usage of a Sparse Distributed Memory (Kanerva, 1988).  

 We have been working on Pentii Kanerva's Sparse Distributed Memory:
   - The first [paper considered the range of dimensions that an SDM should have were it to respect i) chunking-by-averaging, and ii) the "magic number 7"](http://journals.plos.org/plosone/article?id=10.1371/journal.pone.0015592);
   - a second [paper studied the critical distance as the memory becomes saturated](http://journal.frontiersin.org/article/10.3389/fnhum.2014.00222/full);
   - a third paper (underway) studies interaction effects between different attractors, and
   - a fourth paper will document this [highly-palallel SDM framework](https://github.com/msbrogli/sdm-framework) developed by [PhD Candidate Marcelo Brogliato](https://github.com/msbrogli).  

We would really like to ask users for feedback, and, should they find it useful, a link or citation:

Brogliato, M.S.; Linhares, A. (2017) Sparse Distributed Memory: a reference implementation.  Working Paper, FGV, Vialink.


How to build & test
===================
To generate the library and run some tests:

    cd src/
	make
	make tests
	./test1

To run Python tests:

	python tests.py

In dev mode, there should be a symbolic link from `src/libsdm.so` to `sdm/_libsdm.so`. If it does not exist, create one running:

	cd sdm/
	ln -s ../src/libsdm.so _libsdm.so


How to install
==============
This framework has the following dependencies: `libbsd` and `libOpenCL`.

    pip install sdm

If you would like to install the most recent code:

    pip install git+https://github.com/msbrogli/sdm-framework.git

FloydHub
--------
To install in a FloyHub GPU instance, you just have to run:

    apt-get update && apt-get install libbsd-dev nvidia-opencl-icd-304 opencl-headers
    sudo ln -s /usr/lib/x86_64-linux-gnu/libOpenCL.so.1 /usr/lib/libOpenCL.so

AWS GPU instances
-----------------
To install in an AWS instance, you just have to run:

    apt-get update && apt-get install libbsd-dev nvidia-opencl-icd-304 opencl-headers build-essential

For p2 instances:

	wget http://us.download.nvidia.com/XFree86/Linux-x86_64/367.106/NVIDIA-Linux-x86_64-367.106.run
	sudo /bin/bash ./NVIDIA-Linux-x86_64-367.106.run

For p3 instances:

	wget http://us.download.nvidia.com/titan/linux/387.34/nvidia-driver-local-repo-ubuntu1404-387.34_1.0-1_amd64.deb
	dpkg -i nvidia-driver-local-repo-ubuntu1404-387.34_1.0-1_amd64.deb
	sudo apt-key add /var/nvidia-driver-local-repo-387.34/7fa2af80.pub
	dpkg -i nvidia-driver-local-repo-ubuntu1404-387.34_1.0-1_amd64.deb
	apt-get update & apt-get install cuda-drivers
	reboot

To confirm the driver is functional, run `nvidia-smi`.
For further information, see [https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/install-nvidia-driver.html].

To optimize and achieve better performance, run:

	sudo nvidia-persistenced
	sudo nvidia-smi --auto-boost-default=0
	sudo nvidia-smi -ac 2505,875

For further information about optimization, see [https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/optimize_gpu.html].


Documentation
=============
The documentation is available at [http://sdm-framework.readthedocs.io/]


How do I contribute?
====================
Just fork it and do the usual pull request dance. :)


Docker
======
The docker images are published at [https://hub.docker.com/r/msbrogli/sdm-framework/].

They already include the OpenCL ICD for FloydHub GPU instances.

Build
-----
    docker build -t sdm-test ./


Run
---
    docker run -it -p 8888:8888 sdm-test


Useful links
============
- [https://aws.amazon.com/ec2/instance-types/]
- [https://docs.aws.amazon.com/AmazonECR/latest/userguide/docker-basics.html#docker_next_steps]
- [http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/using_cluster_computing.html]
- [http://jackmorrison.me/2014/09/11/CUDA-on-AWS.html]
- [https://github.com/sschaetz/nvidia-opencl-examples/blob/master/OpenCL/src/oclMatVecMul/oclMatVecMul.cl]
- [https://streamhpc.com/blog/2013-04-28/opencl-error-codes/]


TODO
====
- Coverage using gcov? [https://gcc.gnu.org/onlinedocs/gcc/Gcov.html]
- Unit test using catch? [https://github.com/philsquared/Catch]
