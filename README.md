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


Please notice that it is still in development and not ready to be used. :)


Documentation
=============
The documentation is available at [http://sdm-framework.readthedocs.io/]


How do I contribute?
====================
Just fork it and do the usual pull request dance. :)


Links
=====
- [https://aws.amazon.com/ec2/instance-types/]
- [http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/using_cluster_computing.html]
- [http://jackmorrison.me/2014/09/11/CUDA-on-AWS.html]


TODO
====
- Coverage using gcov? [https://gcc.gnu.org/onlinedocs/gcc/Gcov.html]
- Unit test using catch? [https://github.com/philsquared/Catch]
