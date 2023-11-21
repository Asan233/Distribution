/*
 *  gRPC Server
 * */

#ifndef _GRPC_SERVER_H
#define _GRPC_SERVER_H

#include <string>
#include <grpcpp/grpcpp.h>
#include "Distribution.grpc.pb.h"
#include "HttpParser.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using Distribution::RemoteService;
using Distribution::JMessage;
using Distribution::JString;
using Distribution::JReply;

class JsonService final : public Distribution::RemoteService::Service {
public:
    JsonService(HttpParser& h) : httpPa(h){};

    Status GetRemoteJson(ServerContext* context, const JMessage* key,
                         JString* replyJson) override {
        // Get key from the client
        std::string k = key->key();
        // Get ServerJson
        std::string response = httpPa.getValue(k);
        std::cout << response << std::endl;
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
        int cnt = httpPa.DeleteValue(k);
        // Set the return Message
        reply->set_count(cnt);
        // notify the client ok
        return Status::OK;
    }

private:
    HttpParser& httpPa;
};
#endif
