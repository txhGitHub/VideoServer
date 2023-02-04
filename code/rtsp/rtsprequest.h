#ifndef RTSP_REQUEST_H
#define RTSP_REQUEST_H

#include "../buffer/buffer.h"
#include "../log/log.h"

#include <regex>
#include <stdlib.h>      // atoi()
#include <unordered_map>

#define RTSP_PARAM_STRING_MAX 200
#define RTSP_PARAM_STRING_MIN 20

// #include <vector> 
// #include <algorithm> 

enum METHOD{
    OPTIONS = 0,
    DESCRIBE,
    ANNOUNCE,
    GET_PARAMETER,
    PAUSE,
    PLAY,
    RECORD,
    REDIRECT,
    SETUP,
    SET_PARAMETER,
    TEARDOWN
};

enum METHOD_KEY{
    RTSP_URL_ADDR = 0,
    RTSP_CSEQ,
    RTSP_ACCEPT,
    RTSP_USER_AGENT,
    RTSP_TRANSPORT
};

enum REGEX_KEY{
    REG_DES_IP = 0,
    REG_RTSP_URL,
    REG_RTSP_METHODS,
    REG_CSEQ,
    ERG_METHOD_KEY
};
// template <typename T>


class RtspRequest
{

// /
typedef std::function<void (std::string,const char*, const char* , const char*, const int, const int)> CallbackFun;

public:
    /// @brief
    /// @param buff
    /// @return
    bool parse(Buffer &buff, std::string readline);
    void init();
    bool processRequest_Common(std::string line);
    bool processRequest_Setup(std::string line);
    bool processRequest_Play(std::string line);
    void setResponePar(CallbackFun fun);
    // void initMethodKey(METHOD method);
    

    static std::unordered_map<std::string, METHOD>  g_methodMatchTable;
    static  std::unordered_map<REGEX_KEY ,std::string>  g_regMatchTable;
    static std::unordered_map<std::string, METHOD_KEY> g_keyMatchTable;
private:
    //const char *的值，赋值之后的值不可改变，但是可以重新赋值。
    //char * 不能直接使用字符串初始化，字符串初始化，新建的字符串相当于const，因此，
    //一旦改变，就会报错。
// 不要尝试把局部变量赋值给成员变量，局部变量在生命周期结束后，成员变量指针会为空
    char m_CSeq[RTSP_PARAM_STRING_MIN];
    std::string m_curReqMethod;
    const char* m_rtspURL;

    char m_preSuffix[RTSP_PARAM_STRING_MAX];
    char m_suffix[RTSP_PARAM_STRING_MAX];
    int m_rtpPort = 0;
    int m_rtcpPort = 0;
};
#endif 