#include "../buffer/buffer.h"
#include <regex>
#include <stdlib.h>      // atoi()
#include "../log/log.h"

#define RTSP_PARAM_STRING_MAX 200

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

enum REGEX_KEY{
    RTSP_DES_IP = 0,
    RTSP_METHODS,
    CSEQ
};



class RtspRequest
{

public:
    /// @brief
    /// @param buff
    /// @return
    bool parse(Buffer &buff, std::string readline);
    void init();
    bool processOptions(std::string line);

    static std::map<std::string, METHOD>  g_methodMatchTable;
    static  
};