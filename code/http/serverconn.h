/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef SERVER_CONN_H
#define SERVER_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "../rtsp/rtsprequest.h"
#include "../rtsp/rtspresponse.h"
#include "../timer/codetimer.h"

enum PROTOCOL{
    HTTP = 0,
    RTSP = 1
};

class ServerConn {
public:
    ServerConn();

    ~ServerConn();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;
    
    sockaddr_in GetAddr() const;
    
    bool process();
    bool processHttpRequest();
    bool processRtspRequest(std::string line);

    int ToWriteBytes() {
        return iovCnt_ > 1 ? iov_[0].iov_len + iov_[1].iov_len  : iov_[0].iov_len;
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    void matchProtocol();

    

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
    static std::map<std::string, PROTOCOL> g_protocolMatchTable;
    
private:
   
    int fd_;
    struct  sockaddr_in addr_;
    
    bool isClose_;
    
    int iovCnt_;
    struct iovec iov_[2];
    
    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpRequest request_;
    HttpResponse response_;
    PROTOCOL m_protocolType;
    RtspRequest m_rtspRequest;
    RtspReponse m_rtspResponse;
};


#endif //SERVER_CONN_H