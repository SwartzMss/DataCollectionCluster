#ifndef _MQ_CONNECTION_H_
#define _MQ_CONNECTION_H_

#include "libMqDefine.h"
#include <list>

class MqConnection:public ExceptionListener
{
public:
    MqConnection(const string& strBrokerUrl);
    ~MqConnection();

    bool Conn();   //连接
    bool DisConn();//断开连接

    MqProducer* AddProducer(const string& strProducerName, MqMsgMode nType);  //添加生产者
    MqConsumer* AddConsumer(const string& strConsumerName, MqMsgMode nType, MqMsgCb pfnMsgCb,void* pUser);//添加消费者
    bool DelProducer(const string& strProducerName, MqMsgMode nType);//删除生产者，返回连接是否为空
    bool DelConsumer(const string& strConsumerName, MqMsgMode nType);//删除消费者，返回是否连接为空
    bool HasConsumer(const string& strConsumerName, MqMsgMode nType);//判断消费者是否存在

    void DoReconnWork();
private:
    string      m_strBrokerUrl; //Mq连接url
    Connection* m_pConnection;  //连接
    Session*    m_pSession;     //Mq会话

    bool        m_bReConn;    //是否正在重连
    CMutex   m_lockReConn;    //重连线程锁
    swartz_thread_t* m_thrdReConn;	
	swartz_sem_t* m_sem; 	

    CMutex         m_lockProCon;  //队列锁，生产者与消费者基本不会修改，共用一个锁
    list<MqProducer*> m_listProducer;//连接上的生产者队列
    list<MqConsumer*> m_listConsumer;//连接上的消费者队列

    void onException(const CMSException& ex);
    void ReConn();//重连
    bool ReBuildConn();
    void DestroyConn();

    bool IsEmptyConn();
};

#endif//_MQ_CONNECTION_H_