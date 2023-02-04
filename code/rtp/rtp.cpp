#include "rtp.h"

RTP::RTP(const char* path) : m_h264Parser(path)
{   
}

RTP::~RTP()
{
    close(m_rtpSockFd);
    close(m_rtcpSockFd);
}

RTP::RTP_Header::RTP_Header(
            //byte 0
            const uint8_t version,
            const uint8_t padding,
            const uint8_t extension,
            const uint8_t csrcCount,
            //byte 1
            const uint8_t marker,
            const uint8_t payloadType,
            //byte 2, 3
            const uint16_t seq,
            //byte 4-7
            const uint32_t timestamp,
            //byte 8-11
            const uint32_t ssrc)
{
    this->m_Version = version;
    this->m_Padding = padding;
    this->m_Extension = extension;
    this->m_CsrcCount = csrcCount;
    this->m_Marker = marker;
    this->m_PayloadType = payloadType;
    this->m_Seq = htons(seq);
    this->m_Timestamp = htonl(timestamp);
    this->m_Ssrc = htonl(ssrc);
}

//htons（针对16） , htonl (针对32)
RTP::RTP_Header::RTP_Header(uint16_t seq, uint32_t timestamp, uint32_t ssrc)
{
    this->m_Version = RTP_VERSION;
    this->m_Padding = 0;
    this->m_Extension = 0;
    this->m_CsrcCount = 0;

    this->m_Marker = 0;
    this->m_PayloadType = RTP_PAYLOAD_TYPE_H264;
    
    this->m_Seq = seq;
    this->m_Timestamp = timestamp;
    this->m_Ssrc =ssrc;
}

RTP::RTP_Header::RTP_Header()
{
    m_Version = 0;
    m_Padding = 0;
    m_Extension = 0;
    m_CsrcCount = 0;
    m_PayloadType = 0;
    m_Marker = 0;
    m_Seq = 100;
    m_Timestamp = 0;
    //ssrc整个会话唯一
    m_Ssrc = 0;
}

int RTP::RTP_Header::makeRtpHeader(uint8_t* rtpData)
{
    //SSRC数据的传输中，在同一个RTP会话中是全局唯一的
    //一个RTP会话包括传给某个指定目的地对(Destination Pair)的所有通信量。
    int totalSize = 0;
    int u32size = 4;
    
    unsigned header = 0x00000000;
    header <<= 2;
    //byte 0
    header |= (m_Version&0x3);
    header <<= 1;
    header |= (m_Padding&0x1);
    header <<= 1;
    header |= (m_Extension&0x1);
    header <<= 4;
    header |= (m_CsrcCount&0xF);
    //byte 1
    header <<= 1;
    header |= (m_Marker&0x1);
    header <<= 7;
    header |= (m_PayloadType&0x7F);
    //byte2-3
    header <<= 16;
    header |= (m_Seq&0xFFFF);
    //多字节数据需要进行网络字节转换
    //注意，将一个多字节数据转化为char型之后，设置变量保存值大小，墙置换转的数据没有
    //'\0'作为字符串结尾
    uint32_t temHeader = htonl(header);
    unsigned char* word = (unsigned char *) &temHeader;
    // m_h264Parser
    memmove(rtpData, word, u32size);
    totalSize += u32size;

    //设置时间戳,初始化时已经转换过网络字节了，不需再次转换，下同
    uint32_t temTt = htonl(m_Timestamp);
    unsigned char* timestamp = (unsigned char *) &temTt;
    memmove(rtpData + totalSize, timestamp, u32size);
    totalSize += u32size;

    //设置媒体源标识,强制转为char，之后，用数组调用，如tem[0]对应的是低位。
    uint32_t temSs = htonl(m_Ssrc);
    unsigned char* ssrc = (unsigned char *) &temSs;
    memmove(rtpData + totalSize, ssrc, u32size);
    totalSize += u32size;
    return totalSize;
    // 逐步填充bufferd
    //时间戳先默认为0，然后在数据发送时填充
}

int RTP::packRTP(uint8_t* destData, const uint8_t* srcData, const int64_t dataSize, bool isGroup)
{
    //每次打包数据包数据之前，先设置头数据
    // char p_header[RTP_HEADER_SIZE];

    m_seq++; 
    // m_timestamp += 3600;
        
    RTP::RTP_Header rstpHeader(m_seq, m_timestamp, m_ssrc);
    int totalSize = rstpHeader.makeRtpHeader(destData);
    uint8_t* temAddr = destData + totalSize;
    //数据赋值需要确认是否分组发送
    if(!isGroup) {
        memcpy(temAddr , srcData, dataSize);
        return totalSize + dataSize;
    }
    memset(temAddr, 0, FU_Size);
    totalSize += FU_Size; //负载头占用两个字节，分辨标识FU标识符号，和FU头
    memcpy(destData + totalSize, srcData, dataSize);
    // rtpPack.load_data(data + pos, RTP_MAX_DATA_SIZE, FU_Size);
    return totalSize + dataSize;
}

int RTP::pushStream()
{
    //存储RTP数据
    int result = -1;
    int pos = 0;
    struct sockaddr_in clientSock
    {
    };
    bzero(&clientSock, sizeof(sockaddr_in));
    clientSock.sin_family = AF_INET;
    inet_pton(clientSock.sin_family, m_Ip, &clientSock.sin_addr);
    clientSock.sin_port = htons(m_rtpPort);
    LOG_DEBUG("xinhong %d",  m_rtpPort);

    while (true)
    {
        uint8_t rtpData[RTP_MAX_PACKET_LEN] = {0};
        auto cur_frame = m_h264Parser.get_next_frame();
        const auto ptr_cur_frame = cur_frame.first;
        const auto cur_frame_size = cur_frame.second;
        // LOG_DEBUG("xinhong cur_frame_size:%d", cur_frame_size);

        if (cur_frame_size < 0) {
            LOG_ERROR("RTSP::serve_client() H264::getOneFrame() failed\n");
            return result;
        } else if (!cur_frame_size) {
            LOG_DEBUG("Finish serving the user\n");
            result = 0;
            return result;
        }

        //是否需要分组发送
        const int64_t start_code_len = H264Parser::is_start_code(ptr_cur_frame, cur_frame_size, 4) ? 4 : 3;
        const int64_t dataSize = cur_frame_size - start_code_len;
        const int64_t packetNum = (dataSize - 1) / RTP_MAX_DATA_SIZE;
        const int64_t remainPacketSize = (dataSize - 1) % RTP_MAX_DATA_SIZE;
        const u_int8_t* naluHeader = ptr_cur_frame + start_code_len;
        int sentBytes = 0;
        usleep(36000); //增加数据发送的时间
        m_timestamp += 3600;
        if(packetNum == 0)
        {
            pos = 0;
            int bufferLen = packRTP(rtpData, naluHeader + pos, dataSize, packetNum > 0? true : false);
            sentBytes += sendto(m_rtpSockFd, rtpData, bufferLen, 0, (sockaddr *)&clientSock, sizeof(sockaddr));
            memset(rtpData, '\0', sizeof(rtpData));
            // LOG_DEBUG("send byte count:%d", sentBytes);
            continue;
        }

        pos = 1;
        u_int8_t* temData = rtpData + RTP_HEADER_SIZE;
        for(int i = 0; i < packetNum; i++) {
            int bufferLen = packRTP(rtpData, naluHeader + pos, RTP_MAX_DATA_SIZE, packetNum > 0? true : false);
            if (!i)
                temData[1] |= FU_S_MASK;
            else if (i == packetNum - 1 && remainPacketSize == 0)
                temData[1] |= FU_E_MASK;
            temData[0] = (naluHeader[0] & NALU_F_NRI_MASK) | SET_FU_A_MASK;
            temData[1] = (naluHeader[0] & NALU_TYPE_MASK);
            if(m_rtpSockFd > 0) {
                sentBytes += sendto(m_rtpSockFd, rtpData, bufferLen, 0, (sockaddr *)&clientSock, sizeof(sockaddr));
                memset(rtpData, '\0', sizeof(rtpData));
                // if(sentBytes < 0) {
                //     LOG_DEBUG("send failed!");
                //     perror("send error: ");
                // } else{
                //     LOG_DEBUG("send byte count:%d", sentBytes);
                // }
            }
            pos += RTP_MAX_DATA_SIZE;
        }

        if (remainPacketSize > 0)
        {
            int bufferLen = packRTP(rtpData, naluHeader + pos, remainPacketSize, packetNum > 0? true : false);
            temData[0] = (naluHeader[0] & NALU_F_NRI_MASK) | SET_FU_A_MASK;
            temData[1] = (naluHeader[0] & NALU_TYPE_MASK) | FU_E_MASK;
            if(m_rtpSockFd > 0) {
                sentBytes += sendto(m_rtpSockFd, rtpData, bufferLen, 0, (sockaddr *)&clientSock, sizeof(sockaddr));
                memset(rtpData, '\0', sizeof(rtpData));

                // if(sentBytes < 0) {
                //     LOG_DEBUG("send failed!");
                //     perror("send error: ");
                // } else{
                //     LOG_DEBUG("send byte count:%d", sentBytes);
                // }
                //  LOG_DEBUG("xinhong port %d  m_Ip:%d", m_rtpPort, m_Ip);
            }
        }    
    }
}

void RTP::init(const char* ip)
{
    initSocket();
    m_seq = 100;
    m_timestamp = 100;
    //上面后续通过随机数产生或者按照惯例固定值
    m_ssrc = 0x5f4df327;
    memcpy(m_Ip, ip, strlen(ip));
}

bool RTP::bindSocket(int sockfd, const char *IP, const uint16_t port)
{
    sockaddr_in addr{};
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, IP, &addr.sin_addr);
    if (bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        fprintf(stderr, "bind() failed: %s\n", strerror(errno));
        return false;
    }
    return true;
}

int RTP::createSocket(int domain, int type, int protocol)
{
     int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        fprintf(stderr, "RTSP::Socket() failed: %s\n", strerror(errno));
        return sockfd;
    }
    const int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &MAX_UDP_PACKET_SIZE, sizeof(MAX_UDP_PACKET_SIZE)) < 0)
    {
        fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
        return -1;
    }
    return sockfd;
}

bool RTP::setPort(int rtpPort, int rtcpPort)
{
    if(rtpPort <= 0 || rtcpPort <= 0)
        return false;
    m_rtpPort = rtpPort;
    m_rtcpPort = rtcpPort;
    return true;
}

void RTP::initSocket()
{
    //初始化时先将数据设置为0
    m_rtpSockFd = createSocket(AF_INET, SOCK_DGRAM);
    m_rtcpSockFd = createSocket(AF_INET, SOCK_DGRAM);

    if (!bindSocket(m_rtpSockFd, "0.0.0.0", SERVER_RTP_PORT))
    {
        fprintf(stderr, "failed to create RTP socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!bindSocket(m_rtcpSockFd, "0.0.0.0", SERVER_RTCP_PORT))
    {
        fprintf(stderr, "failed to create RTCP socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }   
}