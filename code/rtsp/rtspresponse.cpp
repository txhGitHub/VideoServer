#include "rtspresponse.h"
RtspReponse::~RtspReponse()
{
}

RtspReponse::RtspReponse()
{
}

void RtspReponse::init(const char* ip){
    //获取链接ip
    memcpy(m_Ip, ip, strlen(ip));
}

// Buffer& buff,  METHOD method
void RtspReponse::fillResponseBuffer(Buffer& buff)
{
    unsigned responsesize = strlen(m_responseStr);
    // LOG_ERROR("xinhong m_responseStr:%s", m_responseStr);
    buff.Append(m_responseStr, responsesize);
}

//std::string rtspurl, std::string cseq 
void RtspReponse::makeResponeStr(std::string method , const char* cseq, const char* presuffix, 
            const char* suffix, const int rtpPort, const int rtcpPort)
{
    switch (RtspRequest::g_methodMatchTable[method])
    {
    case OPTIONS:
        snprintf((char*)m_responseStr, sizeof m_responseStr,
        "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
        cseq, dateHeader(), "OPTIONS, DESCRIBE, SETUP, PLAY");
        break;
    case DESCRIBE:  
        {
            char * sdpDescription = generateSDPDescription();
            unsigned sdpDescriptionSize = strlen(sdpDescription);
            snprintf((char*)m_responseStr, sizeof m_responseStr,
            "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
            "%s"
            "Content-Base: %s%s/\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            cseq,
            dateHeader(),
            presuffix,
            suffix,
            sdpDescriptionSize,
            sdpDescription);

            if(suffix != nullptr)
            {
                memcpy(m_filePath, suffix, RTSP_PARAM_STRING_MIN);
            }
        }
        break;
    case SETUP:
        {
            snprintf((char*)m_responseStr, sizeof m_responseStr, 
                "RTSP/1.0 200 OK\r\nCseq: %s\r\nTransport: RTP/AVP/UDP;unicast;client_port=%d-%d;server_port=%d-%d\r\nSession: 138A4393;timeout=65\r\n\r\n",
             cseq, rtpPort, rtcpPort, SERVER_RTP_PORT, SERVER_RTCP_PORT);
            m_rtpPort = rtpPort;
            m_rtcpPort = rtcpPort;
        }
        break;
    case PLAY:
        {
            snprintf((char*)m_responseStr, sizeof m_responseStr,
                "RTSP/1.0 200 OK\r\nCseq: %s\r\nRange: npt=0.000-\r\nSession: 138A4393; timeout=65\r\n\r\n", cseq);
            //单独开辟线程，用于发送RTP包
            std::thread([&](RtspReponse *pointer) {
                        pointer->sendRTPPacket();
                }, this).detach();
        }
        break;
    default:
        break;
    }
}

char const* RtspReponse::dateHeader()
{
    static char buf[200];
    time_t tt = time(NULL);
    strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));
    return buf;
}

char* RtspReponse::generateSDPDescription()
{
    // 暂不适配ipv6
    struct sockaddr_storage ourAddress;
    struct timeval time;
    char* sdp = NULL; // for now
    gettimeofday(&time, NULL);
    do {
        unsigned sdpLength = 0;
        char const *const sdpPrefixFmt =
        "v=0\r\n"           //版本
        "o=- %ld%06ld %d IN %s %s\r\n"   //IN表示intent, 网络类型
        "s=%s\r\n"               
        "i=%s\r\n"
        "t=0 0\r\n"
        "a=tool:%s\r\n"
        "a=type:broadcast\r\n"
        "a=control:*\r\n"
        "%s"
        "a=x-qt-text-nam:%s\r\n"
        "a=x-qt-text-inf:%s\r\n"
        "%s";

        sdpLength = strlen(sdpPrefixFmt) + 20 + 6 + 20 + 3 + 19+200;

        sdp = new char[sdpLength];  //注意释放

        if (sdp == NULL)
            break;
        
        snprintf(sdp, sdpLength, sdpPrefixFmt, time.tv_sec, time.tv_usec,
        1,
        "IP4",
        "192.168.171.1",
        "H.264 Video, streamed by the LIVE555 Media Server",
        "test.264",
        "LIVE555 Streaming Media v2022.12.01",
        "a=range:npt=now-\r\n",
        "H.264 Video, streamed by the LIVE555 Media Server",
        "test.264",
        "");

        // Then, add the (media-level) lines for each subsession:
        char* mediaSDP = sdp;
        unsigned mediaSDPLength = strlen(mediaSDP);
        mediaSDP += mediaSDPLength;

        snprintf(mediaSDP, 400, "%s%s%s%s%s%s",
        "m=video 0 RTP/AVP 96\r\n",
        "c=IN IP4 0.0.0.0\r\n",
        "b=AS:500\r\n",
        "a=rtpmap:96 H264/90000\r\n",
        "a=fmtp:96 packetization-mode=1;profile-level-id=42C015;sprop-parameter-sets=Z0LAFdkAoCOwEQAAAwABAAADADIPFi5I,aMuMsg==\r\n",
        "a=control:track1\r\n");
    } while (0);
    return sdp;
}

char* RtspReponse::getResponseStr()
{
    return m_responseStr;
}

void RtspReponse::sendRTPPacket()
{
    RTP rtp(m_filePath);
    rtp.init(m_Ip);

    if(!rtp.setPort(m_rtpPort, m_rtcpPort))
    {
        LOG_ERROR("set port failed for rtp port!");
    }
    rtp.pushStream();
}

void RtspReponse::resetStr()
{
    memset(m_responseStr,'\0',sizeof(m_responseStr));
}