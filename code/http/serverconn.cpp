/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 
#include "serverconn.h"
using namespace std;

const char* ServerConn::srcDir;
std::atomic<int> ServerConn::userCount;
bool ServerConn::isET;

ServerConn::ServerConn() { 
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

ServerConn::~ServerConn() { 
    Close(); 
};

void ServerConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void ServerConn::Close() {
    response_.UnmapFile();
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int ServerConn::GetFd() const {
    return fd_;
};

struct sockaddr_in ServerConn::GetAddr() const {
    return addr_;
}

const char* ServerConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int ServerConn::GetPort() const {
    return addr_.sin_port;
}

ssize_t ServerConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t ServerConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}

bool ServerConn::process() {
    //根据请求的类型，做不同的处理
    request_.Init();
    const char CRLF[] = "\r\n";

    //使用正则表达式，匹配协议类型，根据不同的协议处理处理不同的请求
    std::string pattern("HTTP|RTSP");
    smatch results;
    regex r(pattern);  

    const char* lineEnd = search(readBuff_.Peek(), readBuff_.BeginWriteConst(), CRLF, CRLF + 2);
    std::string line(readBuff_.Peek(), lineEnd);
    std::cout << "xinhong matching result:" << regex_search(line, results, r) << endl;

    LOG_DEBUG("xinhong %s", results.str().c_str());


    if(readBuff_.ReadableBytes() <= 0) {

        return false;
    }
    else if(request_.parse(readBuff_)) {
        LOG_DEBUG("%s", request_.path().c_str());
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(srcDir, request_.path(), false, 400);
    }


    response_.MakeResponse(writeBuff_);
    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    /* 文件 */
    if(response_.FileLen() > 0  && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;
}
