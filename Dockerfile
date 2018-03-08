
FROM ubuntu:16.04

RUN useradd -ms /bin/bash ubuntu

RUN apt-get update -y
RUN apt-get install -y libbsd0 libbsd-dev opencl-headers python-pip
RUN apt-get install -y ocl-icd-opencl-dev
RUN apt-get install -y nvidia-opencl-icd-304

RUN pip install --upgrade pip
RUN pip install numpy scipy matplotlib ipython jupyter pandas sympy nose pillow
RUN pip install git+https://github.com/msbrogli/sdm-framework.git

COPY docs/notebooks/*.ipynb /home/ubuntu/examples/

EXPOSE 8888

USER ubuntu
WORKDIR /home/ubuntu
CMD ["jupyter", "notebook", "--ip=0.0.0.0"]
