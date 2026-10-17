#include "http.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>

namespace andros {

constexpr size_t kMaxRequest = 1 << 20;

bool WriteAll(int fd, const std::string& data) {
    size_t off = 0;
    while (off < data.size()) {
        ssize_t n = send(fd, data.data() + off, data.size() - off, MSG_NOSIGNAL);
        if (n <= 0) return false;
        off += size_t(n);
    }
    return true;
}

std::string UrlDecode(const std::string& in) {
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '+') out += ' ';
        else if (in[i] == '%' && i + 2 < in.size()) {
            out += char(strtol(in.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        } else out += in[i];
    }
    return out;
}

std::string QueryParam(const std::string& query, const std::string& key) {
    size_t pos = 0;
    while (pos < query.size()) {
        size_t amp = query.find('&', pos);
        std::string pair = query.substr(pos, amp == std::string::npos ? amp : amp - pos);
        if (pair.compare(0, key.size() + 1, key + "=") == 0) return UrlDecode(pair.substr(key.size() + 1));
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

bool ReadRequest(int fd, Request* req) {
    std::string raw;
    char buf[4096];
    size_t header_end = std::string::npos;
    while (header_end == std::string::npos && raw.size() < kMaxRequest) {
        ssize_t n = recv(fd, buf, sizeof buf, 0);
        if (n <= 0) return false;
        raw.append(buf, size_t(n));
        header_end = raw.find("\r\n\r\n");
    }
    if (header_end == std::string::npos) return false;
    size_t line_end = raw.find("\r\n");
    std::string line = raw.substr(0, line_end);
    size_t sp1 = line.find(' '), sp2 = line.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return false;
    req->method = line.substr(0, sp1);
    std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
    size_t q = target.find('?');
    req->path = target.substr(0, q);
    req->query = q == std::string::npos ? "" : target.substr(q + 1);

    size_t length = 0;
    size_t cl = raw.find("Content-Length:");
    if (cl == std::string::npos) cl = raw.find("content-length:");
    if (cl != std::string::npos && cl < header_end) length = size_t(atol(raw.c_str() + cl + 15));
    if (length > kMaxRequest) return false;
    req->body = raw.substr(header_end + 4);
    while (req->body.size() < length) {
        ssize_t n = recv(fd, buf, sizeof buf, 0);
        if (n <= 0) return false;
        req->body.append(buf, size_t(n));
    }
    return true;
}

void Respond(int fd, int status, const std::string& type, const std::string& body) {
    const char* text = status == 200 ? "OK" : status == 400 ? "Bad Request" : "Not Found";
    std::string head = "HTTP/1.1 " + std::to_string(status) + " " + text + "\r\nContent-Type: " +
                       type + "\r\nContent-Length: " + std::to_string(body.size()) +
                       "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
    WriteAll(fd, head + body);
}

}  // namespace andros
