#include "rtsprequest.h"

std::map<std::string, METHOD> RtspRequest::g_methodMatchTable = {
    {"OPTIONS", OPTIONS},
    {"DESCRIBE", DESCRIBE},
    {"GET_PARAMETER", GET_PARAMETER},
    {"PAUSE", PAUSE},
    {"PLAY", PLAY},
    {"REDIRECT", REDIRECT},
    {"SET_PARAMETER", SET_PARAMETER},
    {"TEARDOWN", TEARDOWN},
    {"ANNOUNCE", ANNOUNCE},
    {"RECORD", RECORD},
    {"SETUP", SETUP}
};


std::map<REGEX_KEY ,std::string>  g_regMatchTable = {
    {RTSP_DES_IP, "([r][t][s][p][:][/][/])?((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2}) \
                            (\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}:[0-9a-zA-Z]*"},
    {RTSP_METHODS, "OPTIONS|DESCRIBE|ANNOUNCE|GET_PARAMETER|PAUSE|PLAY|RECORD| \
                            REDIRECT|SETUP|SET_PARAMETER|TEARDOWN"},
    {CSEQ , "([C][S][e][q][:]([ ])?[0-9a-zA-Z]*)"},
            
}

bool RtspRequest::parse(Buffer &buff, std::string readline)
{
    const char CRLF[] = "\r\n"; 
    METHOD cur_req_method = OPTIONS;
    std::smatch results;
    std::regex r(g_regMatchTable[RTSP_METHODS]);

    if(buff.ReadableBytes() <= 0) {
        return false;
    }

    if(regex_search(readline, results, r))
    {
        cur_req_method = g_methodMatchTable[results.str()];
    }

    while (buff.ReadableBytes())
    {
        //获取每一行数据
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);

        //解析每一行数据
        switch (cur_req_method)
        {
        case OPTIONS:
            processOptions(line);
            LOG_DEBUG("parse OPTIONS cmd");
            break;
        case DESCRIBE:
            LOG_DEBUG("parse DESCRIBE cmd");
            break;
        case ANNOUNCE:
            LOG_DEBUG("parse ANNOUNCE cmd");
            break;
        case GET_PARAMETER:
            LOG_DEBUG("parse GET_PARAMETER cmd");
            break;
        case PAUSE:
            LOG_DEBUG("parse PAUSE cmd");
            break;
        case PLAY:
            LOG_DEBUG("parse PLAY cmd");
            break;
        case RECORD:
            LOG_DEBUG("parse RECORD cmd");
            break;
        case REDIRECT:
            LOG_DEBUG("parse REDIRECT cmd");
            break;
        case SETUP:
            LOG_DEBUG("parse SETUP cmd");
            break;
        case SET_PARAMETER:
            LOG_DEBUG("parse SET_PARAMETER cmd");
        case TEARDOWN:
            LOG_DEBUG("parse TEARDOWN cmd");
            break;
        default:
            LOG_DEBUG(" No corresponding method found");
            break;
        }
        
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }

    // if(lineEnd == buff.BeginWrite()) { break; }
    // buff.RetrieveUntil(lineEnd + 2);
    // const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
    return true;
}

void RtspRequest::init()
{
    
}

bool RtspRequest::processOptions(std::string line)
{
    LOG_DEBUG("xinhong  %s", line.c_str());
    //使用正则匹配rtsp请求地址
    std::regex re(g_regMatchTable[RTSP_DES_IP]);
    std::smatch results;

    if(regex_search(line, results, re))
    {
        LOG_DEBUG(" xinhong results %s", results.str().c_str());
    }
    return true;
}