#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "Distribution.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using Distribution::RemoteService;
using Distribution::JMessage;
using Distribution::JString;
using Distribution::JReply;

class gRPCClient{
public:
    gRPCClient( std::shared_ptr<Channel> channel) : stub_(RemoteService::NewStub(channel)) {
    }

    std::string sendJsonRequest(std::string key){
        JMessage Jrequest;
        JString Jreply;
        std::string json;
        ClientContext context;
        Jrequest.set_key(key);
        Status status = stub_->GetRemoteJson(&context, Jrequest, &Jreply);
        if(status.ok()){
            json = Jreply.json();
            return json;
        }else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return "error";
        }
    }

    int removeRemoteJson(std::string key){
        int cnt = 0;
        JMessage Jrequest;
        JReply reply;
        ClientContext context;
        Jrequest.set_key(key);
        Status status = stub_->DeleteRemoteJson(&context, Jrequest, &reply);
        if(status.ok()){
            cnt = reply.count();
        }else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        }
        return cnt;
    }
private:
    std::unique_ptr<RemoteService::Stub> stub_;
};

/*
void Run() {
    std::string address("localhost:50051");
    gRPCClient client( grpc::CreateChannel(address,grpc::InsecureChannelCredentials()) );
    std::string key = "myname";
    std::string json = "";
    json = client.sendJsonRequest(key);
    client.removeRemoteJson(key);
    client.sendJsonRequest(key);
    std::cout << "Answer received : " << json << std::endl;
}
*/
#endif