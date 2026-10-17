// Tiny blocking HTTP/1.1 helpers (one request per connection).
#pragma once
#include <string>

namespace andros {

struct Request {
    std::string method;
    std::string path;
    std::string query;  // raw query string
    std::string body;
};

bool ReadRequest(int fd, Request* req);
bool WriteAll(int fd, const std::string& data);
void Respond(int fd, int status, const std::string& type, const std::string& body);
std::string UrlDecode(const std::string& in);
std::string QueryParam(const std::string& query, const std::string& key);

}  // namespace andros
