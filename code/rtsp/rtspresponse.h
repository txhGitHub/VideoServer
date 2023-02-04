#ifndef RTSP_RESPONSE_H
#define RTSP_RESPONSE_H

#include "../buffer/buffer.h"
#include "rtsprequest.h"
#include "../rtp/rtp.h"



#include <functional>
#include <string>
#include <sys/socket.h>



#ifndef RESPONSE_BUFFER_SIZE
#define RESPONSE_BUFFER_SIZE 1024
#endif

class RtspReponse
{
private:
    /* data */
public:
    RtspReponse();
    ~RtspReponse();
    void init(const char* ip);
    // Buffer& buff, METHOD method
    void fillResponseBuffer(Buffer& buff);
    void makeResponeStr(std::string method, const char* cseq, const char* presuffix, 
                const char* suffix, int rtpPort, int rtcpPort);
    char const* dateHeader();
    char* generateSDPDescription();
    char* getResponseStr();
    void sendRTPPacket();
    void resetStr();

private:
    char m_responseStr[RESPONSE_BUFFER_SIZE];
    char m_filePath[RTSP_PARAM_STRING_MIN];
    char m_Ip[RTSP_PARAM_STRING_MIN];
    int  m_rtpPort;
    int  m_rtcpPort;
};

#endif 