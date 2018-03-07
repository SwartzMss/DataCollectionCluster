#ifndef _COMM_MGR_DEFINE_H_
#define _COMM_MGR_DEFINE_H_

#include <string>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/util/Date.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include "activemq/commands/ActiveMQTempQueue.h"
#include "activemq/commands/ActiveMQTempTopic.h"

#include "swartz_singleton.h"
#include "swartz_init.h"
#include "swartz_thread.h"
#include "swartz_threadpool.h"
#include "swartz_types.h"
#include "swartz_sem.h"
#include "swartz_mutexEx.h"

#include "mqLogDefine.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace activemq::commands;

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;


enum MqMsgMode
{
    MQ_MODE_TOPIC,/*Topic*/
    MQ_MODE_QUEUE/*Queue*/
};


//�����߽ṹ��ָ
class MqProducer 
{
public:
    MqProducer():m_pDestination(NULL),m_pProducer(NULL),m_pSession(NULL),m_nMode(MQ_MODE_TOPIC){}
    
	Destination*     m_pDestination;   //������Ŀ��ָ��
    MessageProducer* m_pProducer;      //������ָ��
    Session*         m_pSession;       //Mq�Ự
    MqMsgMode        m_nMode;
    string           m_strName;         //������Mq���ƣ����磺usp.all.user.topic

    int SendMsg(const string strMsg);
    void Create(Session* pSession);
    void Destroy();

};

//�����߽ṹ��
class MqConsumer:public MessageListener 
{
public:
    MqConsumer():m_pDestination(NULL),m_pConsumer(NULL),m_pSession(NULL),m_nMode(MQ_MODE_TOPIC),m_pMsgCb(NULL),m_pUser(NULL){}

    Destination*     m_pDestination;     //������Ŀ��ָ��
    MessageConsumer* m_pConsumer;        //������ָ��
    Session*         m_pSession;         //Mq�Ự
    MqMsgMode        m_nMode;
    string           m_strName;          //������Mq����
    MqMsgCb          m_pMsgCb;           //Mq��Ϣ�ص�
    void*            m_pUser;            //�û�ָ��

    void Create(Session* pSession);
    void Destroy();
    void onMessage( const cms::Message* message );
};



#endif