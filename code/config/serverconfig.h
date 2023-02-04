/*
 * @Author       : mark
 * @Date         : 2020-06-28
 * @copyleft Apache 2.0
 */ 
//单例模式，私有化构造函数
//对外提供一个接口
//懒汉模式存在线程安全问题
//一般情况下，单例使用了new,没有使用delete，会导致内存泄漏
//针对线程和内存泄漏问题分别使用：
//1、双检锁    2、智能指针解决

#include<sys/socket.h>


class ServerConfig{

public:
    //利用原理：当线程在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待舒适化结束。
    static ServerConfig* GetInstance()
    {
        static ServerConfig  m_pInstance;
        return &m_pInstance;
    }

    ServerConfig(const ServerConfig&)=delete;
    ServerConfig& operator=(const ServerConfig&)=delete;

private:
    ServerConfig();
    
    
    // static ServerConfig* m_pInstance = new ServerConfig();
};
