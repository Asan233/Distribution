# README

电子科技大学2023分布式大作业：简易分布式缓存系统

## 1.部署方法

​	在有`dockerfile`，`docker-compose`目录下部署`image`

```shell
sudo docker-compose up --build -d
```

​	使用测试脚本，只启用了3个cacheserver，因此参数应为`3`，分别占用宿主机`9527, 9528, 9529`端口。

```shell
./sdcs-test.sh 3
```



## 2.Dockefile

```dockerfile
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
```



## 3. docker-compose

​		配置`container`局域网将三个服务容器放入同一个局域网。

```yaml
version: '3.6'

networks:
  default:
    external: true
    name: web_net

services:
  cacheserver1:
    container_name: cacheServer1
    build: .
    ports:
      - "9527:9527"
    expose:
      - "50051"
      - "50052"
    networks:
      default:

  cacheserver2:
    container_name: cacheServer2
    build: .
    ports:
      - "9528:9527"
    expose:
      - "50051"
      - "50052"
    networks:
      default:

  cacheserver3:
    container_name: cacheServer3
    build: .
    ports:
      - "9529:9527"
    expose:
      - "50051"
      - "50052"
    networks:
      default:
```

