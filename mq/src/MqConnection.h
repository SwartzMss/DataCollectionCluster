#ifndef _MQ_CONNECTION_H_
#define _MQ_CONNECTION_H_

#include "libMqDefine.h"
#include <list>

class MqConnection:public ExceptionListener
{
public:
    MqConnection(const string& strBrokerUrl);
    ~MqConnection();

    bool Conn();   //����
    bool DisConn();//�Ͽ�����

    MqProducer* AddProducer(const string& strProducerName, MqMsgMode nType);  //���������
    MqConsumer* AddConsumer(const string& strConsumerName, MqMsgMode nType, MqMsgCb pfnMsgCb,void* pUser);//���������
    bool DelProducer(const string& strProducerName, MqMsgMode nType);//ɾ�������ߣ����������Ƿ�Ϊ��
    bool DelConsumer(const string& strConsumerName, MqMsgMode nType);//ɾ�������ߣ������Ƿ�����Ϊ��
    bool HasConsumer(const string& strConsumerName, MqMsgMode nType);//�ж��������Ƿ����

    void DoReconnWork();
private:
    string      m_strBrokerUrl; //Mq����url
    Connection* m_pConnection;  //����
    Session*    m_pSession;     //Mq�Ự

    bool        m_bReConn;    //�Ƿ���������
    CMutex   m_lockReConn;    //�����߳���
    swartz_thread_t* m_thrdReConn;	
	swartz_sem_t* m_sem; 	

    CMutex         m_lockProCon;  //���������������������߻��������޸ģ�����һ����
    list<MqProducer*> m_listProducer;//�����ϵ������߶���
    list<MqConsumer*> m_listConsumer;//�����ϵ������߶���

    void onException(const CMSException& ex);
    void ReConn();//����
    bool ReBuildConn();
    void DestroyConn();

    bool IsEmptyConn();
};

#endif//_MQ_CONNECTION_H_