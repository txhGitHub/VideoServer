#include "../buffer/buffer.h"
#include "../log/log.h"
#include "h264parser.h"

#include <arpa/inet.h>
#include<stdlib.h>
#include<sys/time.h>

/**
 * @brief 
 * 注意：RTP包的长度需要小于MTU的最大长度，IP协议中MTU的最大长度为1500字节，除去IP报头（20字节），
 * UDP报头（8字节），所有RTP有效载荷（即NALU内容）的长度不得超过1460字节。
 * RTP打包模式：单一的NALU模式，分片模式，组合模式。
 * M标记为：如果当前NALU单元是为接入单元最后的NALU单元，那么将该为置为1，或者当前的RTP数据包为一个NALU单元最后的那个分片，
 * M的位置为1，其余情况下为0。
 * 
 * 发送流程：
 *      1、根据不同的NALU包的大小确认使用哪种模式发送RTP包。
 *      2、一副完整的数据需要多个RTP包的传输，如果是视频帧，最后一包的mark位需要置为1，同一帧内时间戳是相同的。
 *      3、
 * 分片规则：
 *      1、长度超过MTU最大值时就需要分片
 *      2、相同的NALU单元和必须使用连续的分片单元，NAL单元必须按RTP顺序装配。
 *      3、STAPs,MTAP不可以被分片。
 *      4、运送FU的RTP时戳被设置成分片NAL单元的NALU时刻。
 *      
 *
 * constexpr指明数据是常量
 */
constexpr int64_t MAX_UDP_PACKET_SIZE = 65536;
constexpr int64_t RTP_VERSION = 2;
constexpr int64_t RTP_HEADER_SIZE = 12;
constexpr int64_t RTP_PAYLOAD_TYPE_H264 = 96;
constexpr int64_t FU_Size = 2;
constexpr int64_t RTP_MAX_DATA_SIZE = MAX_UDP_PACKET_SIZE - 8 - 20 - RTP_HEADER_SIZE - FU_Size;
constexpr int64_t RTP_MAX_PACKET_LEN = RTP_MAX_DATA_SIZE + RTP_HEADER_SIZE + FU_Size;

constexpr uint16_t SERVER_RTP_PORT = 12345;
constexpr uint16_t SERVER_RTCP_PORT = SERVER_RTP_PORT + 1;



class RTP
{
    class RTP_Header
    {
        //定义协议内容可以采用位域的方法标识对应的二进制位内容
    private:
        uint8_t m_Version;
        uint8_t m_Padding;
        uint8_t m_Extension;
        uint8_t m_CsrcCount;
        uint8_t m_PayloadType;
        uint8_t m_Marker;
        uint16_t m_Seq;
        uint32_t m_Timestamp;
        uint32_t m_Ssrc;
    public:
        RTP_Header(
            const uint8_t version,
            const uint8_t padding,
            const uint8_t extension,
            const uint8_t csrcCount,
            const uint8_t marker,
            const uint8_t payloadType,
            const uint16_t seq,
            const uint32_t timestamp,
            const uint32_t ssrc 
        );

        RTP_Header(uint16_t seq, uint32_t timestamp, uint32_t ssrc);
        RTP_Header();
        // ~RTP_Header();
        void set_timestamp(const uint32_t _newtimestamp);
        void set_ssrc(const uint32_t SSRC);
        void set_seq(const uint32_t _seq);
        int makeRtpHeader(uint8_t* p_header);
        void *get_header() const;
        uint32_t get_timestamp() const;
        uint32_t get_seq() const;
    };

private:
    int m_rtpSockFd{-1}, m_rtcpSockFd{-1};
    int m_rtpPort;
    int m_rtcpPort;
    uint16_t m_seq;
    uint32_t m_ssrc;
    H264Parser m_h264Parser;
    uint32_t m_timestamp;
    char m_Ip[20];
public:
    RTP(const char* path);
    ~RTP();
    void init(const char* ip);
    void initSocket();
    int packRTP(uint8_t* destData,const uint8_t* srcData, const int64_t dataSize, bool isGroup);
    int pushStream();
    int createSocket(int domain, int type, int protocol = 0);
    bool bindSocket(int sockfd, const char *IP, const uint16_t port);
    bool setPort(int rtpPort, int rtcpPort);
};