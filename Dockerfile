FROM ubuntu:20.04
MAINTAINER Distribution work of Asan for UESTC

# 安装必要编译工具, 由于国内apt缓慢，直接使用apt update error returns code 100 ,则需要换源
RUN sed -i "s@/archive.ubuntu.com/@/mirrors.tuna.tsinghua.edu.cn/@g" /etc/apt/sources.list \
    && rm -Rf /var/lib/apt/lists/* \
    && apt-get update \
    && apt-get install -y gcc \
    && apt-get install -y make \
    && apt-get install -y cmake \
    && apt-get install -y build-essential \
    && apt-get install -y git
#创建RPC目录
RUN mkdir /home/ubuntu && \
    mkdir /home/ubuntu/linux
#将项目程序放入docker中的/server/RPC文件夹
COPY ./RPC  /home/ubuntu/linux/RPC
COPY ./cmake /home/ubuntu/linux/cmake

#COPY /home/ubuntu/grpc /home/ubuntu/linux/grpc
# 设置utf-8语言
ENV LANG C.UTF-8

WORKDIR /home/ubuntu/linux/

RUN git clone https://gitee.com/chenwr2020/grpc.git

WORKDIR /home/ubuntu/linux/grpc/

COPY ./.gitmodules /home/ubuntu/linux/grpc/

RUN git submodule update --init
RUN mkdir -p cmake/build && \
    cd cmake/build && \
    cmake ../.. && \
    make install

# 设置工作目录
WORKDIR /home/ubuntu/linux/RPC/build
RUN cmake .. \
    && make all

CMD ["./server", "0.0.0.0:50051", "9527"]