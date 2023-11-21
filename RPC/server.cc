#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<thread>
#include "RPCServer.h"

#define PORT 9527
#define BUFFER_SIZE 4096

#define SERVERADDR_1 "cacheServer1:50051"
#define SERVERADDR_2 "cacheServer2:50051"
#define SERVERADDR_3 "cacheServer3:50051"

HttpParser httpPareser;

// 返回响应的http请求
std::string handleRequest(const std::string &method, HttpParser &httpParser, gRPCClient& client_1, gRPCClient& client_2, gRPCClient& client_3)
{
    std::string response;
    std::string key;
    std::string t;
    response = "HTTP/1.1 200 OK\r\n";
    response += "Content-type: application/json;charset=UTF-8 \r\n";
    response += "\r\n";
    if(method == "POST"){
        response += "ok";
    }else if(method == "GET"){
        key = httpParser.getKey();
        t = client_1.sendJsonRequest(key);
        if( t == "404, not found" ) t = client_2.sendJsonRequest(key);
        if( t == "404, not found" ) t = client_3.sendJsonRequest(key);
        if(t == "404, not found")
        {
            response = "HTTP/1.1 404\r\n";
            response += "Content-type: application/json;charset=UTF-8 \r\n";
            response += "\r\n";
        }
        else return response += t;
    }else if(method == "DELETE"){
        key = httpParser.getKey();
        int cnt = client_1.removeRemoteJson(key);
        cnt += client_2.removeRemoteJson(key);
        cnt += client_3.removeRemoteJson(key);
        response += std::to_string(cnt);
    }
    response.push_back('\n');
    return response;
}

// 处理客户端请求
void handleClient(int client_socket, gRPCClient& c_1, gRPCClient& c_2, gRPCClient& c_3){
    char buffer[BUFFER_SIZE];
    std::string request;
    int n = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if(n <= 0) return;
    buffer[n] = '\0';
    httpPareser.ParserHttp(buffer, c_1, c_2, c_3);
    std::string response = handleRequest(httpPareser.getMethod(), httpPareser, c_1, c_2, c_3);
    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
}

// Start the gRPC server
void rpcRun(char *post){
    // gRPC server address
    std::string address (post);
    // Create gRPC server
    JsonService service(httpPareser);
    ServerBuilder builder;
    // Build the gRPC
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "gRPC server start listening on port : " << address << std::endl;
    server->Wait();
}

int main(int argc, char** argv){
    int f = 0;
    if(argc != 3)
    {
        std::cout << "Form ./server localhost:post post" << std::endl;
        return 0;
    }
    int  listenfd, connfd;
    struct sockaddr_in  servaddr;
    struct sockaddr_in  clieaddr;
    // 开启RPC服务器
    std::thread rpcServer(rpcRun, argv[1]);
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    memset( &servaddr, 0, sizeof(servaddr) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[2]));

    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if( listen(listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    printf("======waiting for client's request======\n");
    std::cout << "Wait the user on post : " << atoi(argv[2]) << std::endl;
    while(1){
        socklen_t clientadd_len = sizeof(clieaddr);
        if( (connfd = accept(listenfd, (struct sockaddr*)&clieaddr, &clientadd_len) ) == -1) {
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }
        gRPCClient server_1 = ( grpc::CreateChannel(SERVERADDR_1,grpc::InsecureChannelCredentials()) );
        gRPCClient server_2 = ( grpc::CreateChannel(SERVERADDR_2,grpc::InsecureChannelCredentials()) );
        gRPCClient server_3 = ( grpc::CreateChannel(SERVERADDR_3,grpc::InsecureChannelCredentials()) );
        handleClient(connfd, server_1, server_2, server_3);
    }
    close(listenfd);
    return 0;
}