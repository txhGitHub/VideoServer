#include "rtsprequest.h"

std::unordered_map<std::string, METHOD> RtspRequest::g_methodMatchTable = {
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

std::unordered_map<std::string, METHOD_KEY> RtspRequest::g_keyMatchTable= {
    {"rtsp:\\/\\/", RTSP_URL_ADDR},
    {"CSeq:", RTSP_CSEQ},
    {"Accept:", RTSP_ACCEPT},
    {"User-Agent:", RTSP_USER_AGENT},
    {"Transport:", RTSP_TRANSPORT},
};


    // {REG_RTSP_METHODS, "([/])([0-9a-zA-Z])+(([/])+([0-9a-zA-Z](\\.)*)*)+"},
std::unordered_map<REGEX_KEY ,std::string>  RtspRequest::g_regMatchTable = {
    {REG_DES_IP, "([r][t][s][p][:][/][/])((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}:[0-9a-zA-Z]*"},
    {REG_RTSP_METHODS, "OPTIONS|DESCRIBE|ANNOUNCE|GET_PARAMETER|PAUSE|PLAY|RECORD|REDIRECT|SETUP|SET_PARAMETER|TEARDOWN"},
    {REG_CSEQ , "([C][S][e][q][:]([ ])?[0-9a-zA-Z]*)"},
    {ERG_METHOD_KEY , "CSeq:|rtsp:\\/\\/|Accept:|User-Agent:|Transport:"}
};


bool RtspRequest::parse(Buffer &buff, std::string readline)
{
    const char CRLF[] = "\r\n"; 
    std::smatch results;
    std::regex r(g_regMatchTable[REG_RTSP_METHODS]);
    if(buff.ReadableBytes() <= 0) {
        return false;
    }

    if(regex_search(readline, results, r))
    {
        m_curReqMethod = results.str();
    }

    while (buff.ReadableBytes())
    {
        //获取每一行数据
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);
        LOG_DEBUG("xinhong ----line: %s", line.c_str());
        //解析每一行数据
        if(processRequest_Common(line))
        {
            if(lineEnd == buff.BeginWrite()) { break; }
            buff.RetrieveUntil(lineEnd + 2);
            continue;
        };

        switch (g_methodMatchTable[m_curReqMethod.c_str()])
        {
        case OPTIONS:
            // initMethodKey(OPTIONS);
            // processRequest_Common(line);
            break;
        case DESCRIBE:
            // processRequest_Common(line);
            // LOG_DEBUG("parse DESCRIBE cmd");
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
            // processRequest_Play(line);
            LOG_DEBUG("parse PLAY cmd");
            break;
        case RECORD:
            LOG_DEBUG("parse RECORD cmd");
            break;
        case REDIRECT:
            LOG_DEBUG("parse REDIRECT cmd");
            break;
        case SETUP:
            {
                processRequest_Setup(line);
            }
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
    return true;
}

void RtspRequest::init()
{
    memset(m_CSeq,'\0',sizeof(m_CSeq));
    m_curReqMethod = "";
    m_rtspURL = nullptr;
    memset(m_preSuffix,'\0',sizeof(m_CSeq));
    memset(m_suffix,'\0',sizeof(m_CSeq));
}

bool RtspRequest::processRequest_Common(std::string line)
{
    bool result = false;
    std::regex re(g_regMatchTable[ERG_METHOD_KEY]);
    std::smatch results;
    if(!regex_search(line, results, re)) {
        return result;
    }

    switch (g_keyMatchTable[results.str()])
    {
    case RTSP_URL_ADDR:
        //不允许跨过变量的初始化语句直接跳转到该变量作用域的另一个位置
        {
            bool start_copy_presuf = false;
            bool start_copy_suf = false;
            unsigned n = 0;
            unsigned k = 0;
            //获取到错误的url可能出现地址越界问题
            for (size_t i = 0; i < line.size(); i++) {
                if(line[i] == 'r' && line[i+1] == 't' && line[i+2] == 's' && line[i+3] == 'p' 
                && line[i+4] == ':') {
                    start_copy_presuf = true;
                }

                //通过斜杠区分域名和资源路径,也就是url前缀和suffix
                if(start_copy_presuf == true && (line[i] == '/' && line[i+1] != '/' &&  line[i-1] != '/')) {
                    m_preSuffix[n++] = line[i];
                    start_copy_suf = true;
                    start_copy_presuf = false;
                    continue;
                }

                if(start_copy_suf && line[i] == ' ') {
                    break;
                }
                if(start_copy_presuf) {
                    m_preSuffix[n++] = line[i];
                }

                if(start_copy_suf) {
                    m_suffix[k++] = line[i];
                }
            }
            m_preSuffix[n] = '\0';
            m_suffix[k] = '\0';
            result = true;
        }
        break;
    case RTSP_CSEQ:
        {   unsigned n = 0;
            for (size_t i = 0; i < line.size();) {
                //匹配 "CSeq:"
                if((line[i] == 'C' && line[i+1] == 'S' && line[i+2] == 'e' && line[i+3] == 'q' 
                                && line[i+4] == ':') || line[i] == ' ') {
                    if(line[i] == ' ') {
                        i = i + 1;
                        continue;
                    }
                    i = i + 5;
                    continue;
                }
                //字符串一般形式为：CSeq: 0\r\n，遇到\r停止
                if(line[i] != '\r') {
                    m_CSeq[n++] = line[i];
                    i++;
                    continue;
                }
                i++;
            }
            m_CSeq[n] = '\0';
            result = true;
        }
        break;
    case RTSP_ACCEPT:
        break;
    case RTSP_USER_AGENT:
        break;
    default:
        break;
    }
    return result;
}


bool RtspRequest::processRequest_Setup(std::string line)
{
    bool result = false;
    std::regex re(g_regMatchTable[ERG_METHOD_KEY]);
    std::smatch results;
    if(!regex_search(line, results, re)) {
        return result;
    }

    switch (g_keyMatchTable[results.str()])
    {
        case RTSP_TRANSPORT:
            {
                sscanf(line.c_str(), "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n", 
                                &m_rtpPort, &m_rtcpPort);
                if(m_rtpPort == 0 || m_rtcpPort == 0)
                {
                    sscanf(line.c_str(), "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", 
                                &m_rtpPort, &m_rtcpPort);
                }
                //如果端口错误，终止会话   
                LOG_DEBUG("xinhong m_rtpPort:%d  m_rtcpPort:%d", m_rtpPort, m_rtcpPort);
                LOG_DEBUG("xinhong str: %s", line.c_str());
            }
            break;
    }
}

bool RtspRequest::processRequest_Play(std::string line)
{
    return false;
}

void RtspRequest::setResponePar(CallbackFun fun)
{
    fun(m_curReqMethod, m_CSeq, m_preSuffix, m_suffix, m_rtpPort, m_rtcpPort);
}