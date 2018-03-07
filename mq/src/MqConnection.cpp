#include "libactivemq.h"
#include "MqConnection.h"

MqConnection::MqConnection(const string& strBrokerUrl):m_pConnection(NULL),m_pSession(NULL),m_bReConn(false)
,m_thrdReConn(NULL),m_sem(NULL)
{
    char szURI[256] = {0};
    snprintf(szURI,sizeof(szURI),"tcp://%s?connectionTimeout=5000&soTimeout=10000&maxReconnectAttempts=1"
        "&wireFormat.maxInactivityDuration=0",strBrokerUrl.c_str());
    m_strBrokerUrl = szURI;
    MQ_DEBUG("Add connection %s",m_strBrokerUrl.c_str());

}

MqConnection::~MqConnection()
{
    MQ_DEBUG("Delete connection %s",m_strBrokerUrl.c_str());
    m_pConnection = NULL;
    m_pSession = NULL;
    m_thrdReConn = NULL;
}

/** @fn    Conn
 *  @brief ����Mq����
 *  @param void
 *  @return 
*/
bool MqConnection::Conn()
{
    try
    {
        auto_ptr<ConnectionFactory> connectionFactory(
            ConnectionFactory::createCMSConnectionFactory( m_strBrokerUrl ) );
        m_pConnection = connectionFactory->createConnection();
        m_pConnection->setExceptionListener(this);
        m_pConnection->start();
        m_pSession = m_pConnection->createSession( Session::AUTO_ACKNOWLEDGE );
    }
    catch(...)
    {
        MQ_ERROR("Connect activemq %s fail",m_strBrokerUrl.c_str());

        if (!m_bReConn)
        {
            ReConn();
        }

        return false;
    }

    return true;
}

/** @fn    DisConn
 *  @brief �Ͽ�Mq���ӣ����������ߡ�������
 *  @param void
 *  @return 
*/
bool MqConnection::DisConn()
{
    MQ_DEBUG("Break connection %s",m_strBrokerUrl.c_str());
    if (m_thrdReConn)
    {
        swartz_thread_wait(m_thrdReConn);
        m_thrdReConn = NULL;
    }

    DestroyConn();
    CGuard<CMutex> g(m_lockReConn);
    for (list<MqProducer*>::iterator itr = m_listProducer.begin(); itr != m_listProducer.end(); ++itr)
    {
        delete (*itr);
    }
    m_listProducer.clear();

    for (list<MqConsumer*>::iterator itr = m_listConsumer.begin(); itr != m_listConsumer.end(); ++itr)
    {
        delete (*itr);
    }
    m_listConsumer.clear();

    return true;

}

/** @fn    AddProducer
 *  @brief ���������
 *  @param [in]strProducerName ���������� 
 *  @param [in]nType ����@MqMsgMode
 *  @return ������ָ��
*/
MqProducer* MqConnection::AddProducer(const string& strProducerName, MqMsgMode nType)
{
    MqProducer* pPro = new(std::nothrow) MqProducer;
    if (pPro)
    {
        pPro->m_strName = strProducerName;
        pPro->m_nMode = nType;
        {
             CGuard<CMutex> g(m_lockReConn);
            m_listProducer.push_back(pPro);//����Mq�Ƿ����ߣ������棬�ڲ������Զ�����
        }
        pPro->Create(m_pSession);
    }

    return pPro;
}

/** @fn    AddConsumer
 *  @brief ���������
 *  @param [in]strConsumerName ���������� 
 *  @param [in]nType ����@MqMsgMode
 *  @param [in]pfnMsgCb ����@MqMsgMode
   *  @param [in]pUser �û�ָ��
 *  @return ������ָ��
*/
MqConsumer* MqConnection::AddConsumer(const string& strConsumerName, MqMsgMode nType, MqMsgCb pfnMsgCb,void* pUser)
{
    MqConsumer* pConsumer = new(std::nothrow) MqConsumer;
    if (pConsumer)
    {
        pConsumer->m_strName = strConsumerName;
        pConsumer->m_nMode = nType;
        pConsumer->m_pMsgCb = pfnMsgCb;
        pConsumer->m_pUser = pUser;
        {
             CGuard<CMutex> g(m_lockReConn);
            m_listConsumer.push_back(pConsumer);
        }
        pConsumer->Create(m_pSession);
    }

    return pConsumer;
}

/** @fn    DelProducer
 *  @brief ɾ��������
 *  @param [in]strProducerName ���������� 
 *  @param [in]nType ����@MqMsgMode
 *  @return ɾ����ǰ�����Ƿ�Ϊ��true-�ǣ�false-��
*/
bool MqConnection::DelProducer(const string& strProducerName, MqMsgMode nType)
{
    
    CGuard<CMutex> g(m_lockReConn);
    for (list<MqProducer*>::iterator itr = m_listProducer.begin(); itr != m_listProducer.end(); ++itr)
    {
        if ((*itr)->m_strName == strProducerName && (*itr)->m_nMode == nType)
        {
            (*itr)->Destroy();
            delete (*itr);
            m_listProducer.erase(itr);
            break;
        }
    }
    return IsEmptyConn();
}

/** @fn    DelConsumer
 *  @brief ɾ��������
 *  @param [in]strConsumerName ���������� 
 *  @param [in]nType ����@MqMsgMode
 *  @return ɾ����ǰ�����Ƿ�Ϊ��true-�ǣ�false-��
*/
bool MqConnection::DelConsumer(const string& strConsumerName, MqMsgMode nType)
{
    CGuard<CMutex> g(m_lockReConn);
    for (list<MqConsumer*>::iterator itr = m_listConsumer.begin(); itr != m_listConsumer.end(); ++itr)
    {
        if ((*itr)->m_strName == strConsumerName && (*itr)->m_nMode == nType)
        {
            (*itr)->Destroy();
            delete (*itr);
            m_listConsumer.erase(itr);
            break;
        }
    }
    return IsEmptyConn();
}

/** @fn    HasConsumer
 *  @brief �ж��������Ƿ����
 *  @param [in]strConsumerName ���������� 
 *  @param [in]nType ����@MqMsgMode
 *  @return true-�ǣ�false-��
*/
bool MqConnection::HasConsumer(const string& strConsumerName, MqMsgMode nType)
{
     CGuard<CMutex> g(m_lockReConn);
    for (list<MqConsumer*>::iterator itr = m_listConsumer.begin(); itr != m_listConsumer.end(); ++itr)
    {
        if ((*itr)->m_strName == strConsumerName && (*itr)->m_nMode == nType)
        {
            return true;
        }
    }

    return false;
}

/** @fn    IsEmptyConn
 *  @brief �����Ƿ�Ϊ��
 *  @param void
 *  @return true-�ǣ�false-��
*/
bool MqConnection::IsEmptyConn()
{
    return (m_listConsumer.empty() && m_listProducer.empty());
}

/** @fn    ReConnCb
 *  @brief �����ص�
 *  @param [in]pUser �û�����ָ��
 *  @return NULL
*/
void CALLBACK ReConnCb(void* pUser)
{
    if (pUser)
    {
        MqConnection* self = reinterpret_cast<MqConnection*>(pUser);
        self->DoReconnWork();
    }

    return ;
}

/** @fn    ReConn
 *  @brief ����Mq�����߳�
 *  @param void
 *  @return
*/
void MqConnection::ReConn()
{
    CGuard<CMutex> g(m_lockReConn);
    if (m_bReConn)//�Ѿ���������ֱ�ӷ���
    {
        return;
    }
    MQ_INFO("Start reconn mq");

    if (m_sem&&m_thrdReConn)//������������֮ǰ�������߳���Դ
    {
        swartz_sem_post_1(m_sem);
        swartz_sem_destroy(m_sem);
        m_sem = NULL;

		swartz_thread_wait(m_thrdReConn);
        m_thrdReConn = NULL;
    }

	if (SWARTZ_OK != swartz_sem_create(&m_sem, 0))
    {
        MQ_ERROR("swartz_sem_create m_sem error");
		m_sem = NULL;
        return ;
    }
	if (SWARTZ_OK != swartz_thread_create(&m_thrdReConn, (void*)ReConnCb, this, 0, 0))
	{
		 MQ_ERROR("Start swartz_thread_create ReConnCb error");
		 m_thrdReConn = NULL;
		 return;
	}
    
    m_bReConn = true;//�����������̻߳�δ����ʱ�ִ�����Mq�쳣�����Դ˴��������߳��ڲ������ñ��
}

/** @fn    DoReconnWork
 *  @brief ִ��Mq��������
 *  @param void
 *  @return
*/
void MqConnection::DoReconnWork()
{
    MQ_INFO("ReConnect mq thread start");
    m_bReConn = true;//�����������̻߳�δ����ʱ�ִ�����Mq�쳣�����Դ˴��������߳��ڲ������ñ��
    do
    {
        MQ_INFO("Connection with mq server is closed,ReConnecting mq server");

        DestroyConn();
        if (ReBuildConn())
        {
            break;
        }
    }while (swartz_sem_wait(m_sem, 45*1000) != SWARTZ_OK);

	CGuard<CMutex> g(m_lockReConn);
    m_bReConn = false;
    MQ_INFO("ReConnect mq thread exit");
}

/** @fn    onException
 *  @brief Mq�����쳣�ص�
 *  @param [in]ex �쳣��Ϣ 
 *  @return
*/
void MqConnection::onException(const CMSException& ex)
{
    MQ_ERROR("Mq connection exception, errmsg %s",ex.getMessage().c_str());
    ReConn();
}

/** @fn    ReBuildConn
 *  @brief ���´���Mq���ӣ����������ߺ�������
 *  @param  void
 *  @return true-�ɹ���false-ʧ��
*/
bool MqConnection::ReBuildConn()
{
    if (Conn() == false)
    {
        return false;
    }

     CGuard<CMutex> g(m_lockReConn);
    for (list<MqProducer*>::iterator itr = m_listProducer.begin(); itr != m_listProducer.end(); ++itr)
    {
        (*itr)->Create(m_pSession);
    }

    for (list<MqConsumer*>::iterator itr = m_listConsumer.begin(); itr != m_listConsumer.end(); ++itr)
    {
        (*itr)->Create(m_pSession);
    }

    return true;
}

/** @fn    DestroyConn
 *  @brief ����Mq�����ߡ������ߣ��Ͽ�Mq����
 *  @param  void
 *  @return 
*/
void MqConnection::DestroyConn()
{
    CGuard<CMutex> g(m_lockReConn);
    for (list<MqProducer*>::iterator itr = m_listProducer.begin(); itr != m_listProducer.end(); ++itr)
    {
        (*itr)->Destroy();
    }

    for (list<MqConsumer*>::iterator itr = m_listConsumer.begin(); itr != m_listConsumer.end(); ++itr)
    {
        (*itr)->Destroy();
    }

    try
    {
        if( m_pSession != NULL )
        {
            delete m_pSession;
        }
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str()); 
    }
    catch ( ... )
    {
        MQ_ERROR("Exception");
    }
    m_pSession = NULL;

    try
    {

        if( m_pConnection != NULL )
        {
            delete m_pConnection;
        }
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str()); 
    }
    catch ( ... )
    {
        MQ_ERROR("Exception");
    }
    m_pConnection = NULL;
}


/** @fn    SendMsg
 *  @brief ������Ϣ
 *  @param [in]��Ϣ����
 *  @return �ɹ�����0�����򷵻�����ֵ
*/
int MqProducer::SendMsg(const string strMsg)
{
    int bRet = -1;
    BytesMessage* msg = NULL;
    try 
    {
        if (m_pSession )
        {
            msg = m_pSession->createBytesMessage((const unsigned char*)strMsg.c_str(),strMsg.size());
            if (m_pProducer)
            {
                m_pProducer->send(msg);
                bRet = 0;
            }
        }
    }
    catch ( ... ) 
    {
        MQ_INFO("Send msg to mq fail");
    }

    if (msg)
    {
        delete msg;
        msg = NULL;
    }
    return bRet;
}

/** @fn    Create
 *  @brief �����ߴ���
 *  @param [in]pSession Mq���ӻỰ
 *  @return 
*/
void MqProducer::Create(Session* pSession)
{
    if (!pSession)
    {
        return;
    }

    m_pSession = pSession;
    try
    {
        if (m_nMode == MQ_MODE_QUEUE)
        {
            m_pDestination = m_pSession->createQueue( m_strName.c_str() );
        }
        else
        {
            m_pDestination = m_pSession->createTopic( m_strName.c_str() );
        }

        m_pProducer = m_pSession->createProducer( m_pDestination );
        m_pProducer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str()); 
    }
    catch ( ... )
    {
        MQ_ERROR("Exception");
    }
}

template<typename T,typename Y>
void ReleaseDst(T& pdst,Y& pCp)
{
    try
    {
        if( pdst ) 
        {
            delete pdst;
        }
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str());
        pdst = NULL;
    }
    catch(...)
    {
        MQ_ERROR("Exception");
        pdst = NULL;
    }
    pdst = NULL;

    try
    {
        if( pCp ) 
        {
            pCp->close();
            {
                delete pCp;
            }
        }
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str());
        pCp = NULL;
    }
    catch(...)
    {
        MQ_ERROR("Exception");
        pCp = NULL;
    }
    pCp = NULL;
}

/** @fn    Destroy
 *  @brief ����������
 *  @param NULL
 *  @return 
*/
void MqProducer::Destroy()
{
    ReleaseDst(m_pDestination,m_pProducer);
    m_pSession = NULL;
}

/** @fn    Create
 *  @brief �����ߴ���
 *  @param [in]pSession Mq���ӻỰ
 *  @return 
*/
void MqConsumer::Create(Session* pSession)
{
    if (!pSession)
    {
        return;
    }

    m_pSession = pSession;
    try
    {
        if (m_nMode == MQ_MODE_QUEUE)
        {
            m_pDestination = m_pSession->createQueue( m_strName.c_str() );
        }
        else
        {
            m_pDestination = m_pSession->createTopic( m_strName.c_str() );
        }

        m_pConsumer = m_pSession->createConsumer( m_pDestination );
        m_pConsumer->setMessageListener( this );
    }
    catch ( CMSException& e ) 
    { 
        MQ_ERROR("%s",e.getMessage().c_str()); 
    }
    catch ( ... )
    {
        MQ_ERROR("Exception");
    }
}

/** @fn    Destroy
 *  @brief ����������
 *  @param NULL
 *  @return 
*/
void MqConsumer::Destroy()
{
    ReleaseDst(m_pDestination,m_pConsumer);
    m_pSession = NULL;
}

/** @fn    onMessage
 *  @brief Mq��Ϣ�ص�����
 *  @param [in]message �ص�����Ϣ
 *  @return 
*/
void MqConsumer::onMessage( const cms::Message* message )
{
    try
    {
        const cms::BytesMessage* msg = dynamic_cast< const cms::BytesMessage* >(message);
        if (msg == NULL)
        {
            return;
        }
        char* pBuf = (char*)msg->getBodyBytes();
        if (m_pMsgCb)
        {
            m_pMsgCb(pBuf,msg->getBodyLength(),m_pUser);
        }
        delete[] pBuf;

    } 
    catch (cms::CMSException& e) 
    {
        MQ_INFO("%s",e.getMessage().c_str());
    }
    catch ( ... )
    {
        MQ_ERROR("Exception");
    }
}