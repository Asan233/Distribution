# README

## 1.安装gRPC（C++）

[ubuntu搭建grpc for c++](https://blog.51cto.com/u_13999641/2913394)

**cmake**

 gRPC for C++ 需要将`cmake`升级到`3.18`及以上的版本

直接安装`cmake`

`apt install cmake`

**gRPC**

[github grpc for c++ 官网安装教程](https://github.com/grpc/grpc/blob/master/BUILDING.md)由于国内访问GitHub十分缓慢或者不能直接访问，使用官网教程安装则非常慢或者直接失败。则可以通过过`gitee`导入`github`的克隆提升安装速度，如果将`GitHub`仓库克隆到`Gitee`直接Google教程，我以及将`gRPC仓库`克隆.

下载`gRPC`源码

```sh
git clone https://gitee.com/chenwr2020/grpc.git 
git checkout -b grpc_v1.28.0-pre3 v1.28.0-pre3
```

由于`gRPC`安装组件时，还是默认从`github`中拉取组件，则需要修改拉去仓库地址，将`gRPC/.gitmodules`修改为

```shell
[submodule "third_party/zlib"]
	path = third_party/zlib
	url = https://gitee.com/chenwr2020/zlib.git
	# When using CMake to build, the zlib submodule ends up with a
	# generated file that makes Git consider the submodule dirty. This
	# state can be ignored for day-to-day development on gRPC.
	ignore = dirty
[submodule "third_party/protobuf"]
	path = third_party/protobuf
	url = https://gitee.com/chenwr2020/protobuf.git
	branch = 3.0.x
[submodule "third_party/gflags"]
	path = third_party/gflags
	url = https://gitee.com/chenwr2020/gflags.git
[submodule "third_party/googletest"]
	path = third_party/googletest
	url = https://gitee.com/chenwr2020/googletest.git
[submodule "third_party/benchmark"]
	path = third_party/benchmark
	url = https://gitee.com/chenwr2020/benchmark.git
[submodule "third_party/boringssl-with-bazel"]
	path = third_party/boringssl-with-bazel
	url = https://gitee.com/chenwr2020/boringssl.git
[submodule "third_party/cares/cares"]
	path = third_party/cares/cares
	url = https://gitee.com/chenwr2020/c-ares.git
	branch = cares-1_12_0
[submodule "third_party/bloaty"]
	path = third_party/bloaty
	url = https://gitee.com/chenwr2020/bloaty.git
[submodule "third_party/abseil-cpp"]
	path = third_party/abseil-cpp
	url = https://gitee.com/chenwr2020/abseil-cpp.git
	branch = lts_2020_02_25
[submodule "third_party/envoy-api"]
	path = third_party/envoy-api
	url = https://gitee.com/chenwr2020/data-plane-api.git
[submodule "third_party/googleapis"]
	path = third_party/googleapis
	url = https://gitee.com/chenwr2020/googleapis.git
[submodule "third_party/protoc-gen-validate"]
	path = third_party/protoc-gen-validate
	url = https://gitee.com/chenwr2020/protoc-gen-validate.git
[submodule "third_party/udpa"]
	path = third_party/udpa
	url = https://gitee.com/chenwr2020/udpa.git
[submodule "third_party/libuv"]
	path = third_party/libuv
	url = https://gitee.com/chenwr2020/libuv.git
```

```shell
// 下载gRPC组件
git submodule update --init
```

编译和安装`gRPC`和`Protocol Buffers`

```shell
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake ../..
make install
```

## 2.编写gRPC服务端与客户端

[gRPC C++ 入门教程](https://leehao.me/gRPC-C-%E5%85%A5%E9%97%A8%E6%95%99%E7%A8%8B/)

**Protoc**

使用`Protoc buff`语法定义`gRPC`要提供的调用函数和数据结构。`Distribution.protoc`：

```protobuf
syntax = "proto3";
// 定义Java类生成的包
option java_package = "distribution.grpc";
// 定义消息的包名，对于C++来说，引用的消息类在该Distribution命名空间下;
package Distribution;

//Define Server
service RemoteService{
  // Get Remote Service Json
  rpc GetRemoteJson (JMessage) returns (JString) {}
  // Delete Remote Service Json
  rpc DeleteRemoteJson(JMessage) returns (JReply) {}
}

//The Remote Json Message containing the Key of Json
message JMessage {
  string key = 1;
};

//The Remote Service return the Json K/V String
message JString{
  string Json = 1;
}

//The Remote Service reply containing the number of delete Json
message JReply{
  int32 count = 1;
}
```

执行`protoc`生成使用语言的代码：

```shell
protoc -I ../../protos --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` Distribution.protoc
protoc -I ../../protos --cpp_out=. Distribution.protoc
//结果生成 Distribution.pb.h  Distribution.pb.cc  Distribution.grpc.pb.h  Distribution.grpc.pb.cc
```

编写自己服务端与客户端代码

**Server**

由于`protoc`以及生成了`c++`代码与头文件，`distribution.grpc.h / .cc`为服务器的代码与头文件，它帮我们实现了`protoc`中定义的数据结构和调用方法，我们只需要在`server`代码中实现`protoc`中的服务器`call`函数。

```cpp
/*
 *  gRPC Server
 * */
#include <string>
#include <grpcpp/grpcpp.h>
#include "Distribution.grpc.pb.h"
#include "../HttpParser.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using Distribution::RemoteService;
using Distribution::JMessage;
using Distribution::JString;
using Distribution::JReply;

extern HttpParser httpParser;

class JsonService final : public Distribution::RemoteService::Service {
public:
    Status GetRemoteJson(ServerContext* context, const JMessage* key,
                         JString* replyJson) override {
        // Get key from the client
        std::string k = key->key();
        // Get ServerJson
        std::string response = httpParser.getValue(k);
        // Set the return Message
        replyJson->set_json(response);

        // Server return ok
        return Status::OK;
    }

    Status DeleteRemoteJson(ServerContext* context, const JMessage* key,
                            JReply* reply) override {
        // Get key from the client
        std::string k = key->key();
        // delete the json
        int cnt = httpParser.DeleteValue(k);
        // Set the return Message
        reply->set_count(cnt);
        // notify the client ok
        return Status::OK;
    }
};

// Start the gRPC server
void Run(){
    // gRPC server address
    std::string address("0.0.0.0:5000");
    // Create gRPC server
    class JsonService service;
    ServerBuilder builder;
    // Build the gRPC
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "gRPC server listening on port : " << address << std::endl;
    server->Wait();
}

int main(){
    Run();
    return 0;
}

```

**CMake**

由于构建一个`gRPC`应用程序十分麻烦，则我们使用官方的`cmake示例`去构建一个`gRPC`应用程序。

