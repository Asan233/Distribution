//
// Created by 16110 on 2023/10/16.
//

#ifndef DISTRIBUTION_HTTPPARSER_H
#define DISTRIBUTION_HTTPPARSER_H

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "client.h"

class HttpParser{
private:
    void cc_Json();
    rapidjson::Document JsonValue, tempJson;
    std::string method;
    std::string path;
public:
    HttpParser()
    {
        JsonValue.SetObject();
    }
    ~HttpParser();
    void ParserHttp(char *msg, gRPCClient& c_1, gRPCClient& c_2, gRPCClient& c_3);
    std::string getMethod();
    std::string getKey();
    std::string getValue(std::string key);
    int DeleteValue(std::string key);
};

void HttpParser::cc_Json() {
    std::cout << "Begin Iter Json : " << std::endl;
    for(rapidjson::Value::ConstMemberIterator itr = JsonValue.MemberBegin(); itr != JsonValue.MemberEnd(); ++itr){
        std::cout << itr->name.GetString() << " : " << itr->value.GetString() << std::endl;
    }
}

void HttpParser::ParserHttp(char *msg, gRPCClient& c_1, gRPCClient& c_2, gRPCClient& c_3){
    std::string buf(msg);
    std::istringstream buf_stream(buf);
    enum parts{
        start_line,
        headers,
        body
    };
    parts part = start_line;
    std::string line;
    std::string body_string;
    while(getline(buf_stream, line)){
        switch (part)
        {
            case start_line:
            {
                std::istringstream line_stream(line);
                std::string tmp;
                line_stream >> tmp;
                if(tmp.find("HTTP") == std::string::npos){
                    method = tmp;
                    line_stream >> tmp;
                    path = tmp;
                    line_stream >> tmp;
                } else{
                    line_stream >> tmp;
                    line_stream >> tmp;
                }
                part = headers;
                break;
            }
            case headers:
            {
                if( line.size() == 1 ){
                    part = body;
                    break;
                }
                auto pos = line.find(":");
                if( pos == std::string::npos )
                    continue;
                std::string tmp1(line, 0, pos);
                std::string tmp2(line, pos + 2);
                break;
            }
            case body:
            {
                body_string.append(line);
                body_string.push_back('\n');
                break;
            }
            default:
                break;
        }
    }
    // 解析Json数据
    if(method == "POST")
    {
        body_string.push_back('\0');
        // 将字符串解析成Json
        tempJson.Parse(body_string.c_str());
        rapidjson::Document::AllocatorType& allocator = JsonValue.GetAllocator();
        // 添加到内存存储
        for(rapidjson::Value::ConstMemberIterator itr = tempJson.MemberBegin(); itr != tempJson.MemberEnd(); ++itr){
            // 删除旧Json
            c_1.removeRemoteJson(itr->name.GetString());
            c_2.removeRemoteJson(itr->name.GetString());
            c_3.removeRemoteJson(itr->name.GetString());
            rapidjson::Value name(rapidjson::kStringType);
            rapidjson::Value value(rapidjson::kStringType);
            name.SetString(itr->name.GetString(), allocator);
            value.SetString(itr->value.GetString(), allocator);
            JsonValue.AddMember( name, value, allocator );
        }
    }
}

HttpParser::~HttpParser(){}

std::string HttpParser::getMethod() {
    return method;
}

std::string HttpParser::getKey() {
    int position = path.find('/');
    int size = path.length() - 1;
    return path.substr(position + 1, size);
}

std::string HttpParser::getValue(std::string key) {
    //cc_Json();
    std::string response;
    // 直接使用JsonValue[key.c_str()]报错
    rapidjson::Value::ConstMemberIterator itr = JsonValue.FindMember(key.c_str());
    if( itr == JsonValue.MemberEnd() ){
        response = "404, not found";
    }else {
        response =  "{ \"" + key + "\"" + ": ";
        rapidjson::StringBuffer strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
        itr->value.Accept(writer);
        response += strbuf.GetString();
        response += " }";
    }
    return response;
}

int HttpParser::DeleteValue(std::string key) {
    int cnt = 0;
    //std::cout << "Delete Json : " << key << std::endl;
    // 查询是否存在 key 对象
    rapidjson::Value::ConstMemberIterator itr = JsonValue.FindMember(key.c_str());
    if(itr == JsonValue.MemberEnd()){
        return cnt;
    }else{
        if( itr->value.IsArray() )cnt = itr->value.Size();
        else cnt = 1;
        //std::cout << "sucess delet : " << key << " " << cnt << std::endl;
        JsonValue.RemoveMember(key.c_str());
        return cnt;
    }
}

#endif //DISTRIBUTION_HTTPPARSER_H
