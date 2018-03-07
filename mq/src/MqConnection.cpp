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
 *  @brief 创建Mq连接
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
 *  @brief 断开Mq连接，销毁生产者、消费者
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
 *  @brief 添加生产者
 *  @param [in]strProducerName 生产者名称 
 *  @param [in]nType 类型@MqMsgMode
 *  @return 生产者指针
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
            m_listProducer.push_back(pPro);//无论Mq是否在线，都保存，内部负责自动重连
        }
        pPro->Create(m_pSession);
    }

    return pPro;
}

/** @fn    AddConsumer
 *  @brief 添加消费者
 *  @param [in]strConsumerName 消费者名称 
 *  @param [in]nType 类型@MqMsgMode
 *  @param [in]pfnMsgCb 类型@MqMsgMode
   *  @param [in]pUser 用户指针
 *  @return 消费者指针
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
 *  @brief 删除生产者
 *  @param [in]strProducerName 生产者名称 
 *  @param [in]nType 类型@MqMsgMode
 *  @return 删除后当前连接是否为空true-是，false-否
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
 *  @brief 删除消费者
 *  @param [in]strConsumerName 消费者名称 
 *  @param [in]nType 类型@MqMsgMode
 *  @return 删除后当前连接是否为空true-是，false-否
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
 *  @brief 判断消费者是否存在
 *  @param [in]strConsumerName 消费者名称 
 *  @param [in]nType 类型@MqMsgMode
 *  @return true-是，false-否
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
 *  @brief 连接是否为空
 *  @param void
 *  @return true-是，false-否
*/
bool MqConnection::IsEmptyConn()
{
    return (m_listConsumer.empty() && m_listProducer.empty());
}

/** @fn    ReConnCb
 *  @brief 重连回调
 *  @param [in]pUser 用户数据指针
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
 *  @brief 创建Mq重连线程
 *  @param void
 *  @return
*/
void MqConnection::ReConn()
{
    CGuard<CMutex> g(m_lockReConn);
    if (m_bReConn)//已经在重连了直接返回
    {
        return;
    }
    MQ_INFO("Start reconn mq");

    if (m_sem&&m_thrdReConn)//不在重连销毁之前的重连线程资源
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
    
    m_bReConn = true;//可能在重连线程还未运行时又触发了Mq异常，所以此处和重连线程内部都设置标记
}

/** @fn    DoReconnWork
 *  @brief 执行Mq重连工作
 *  @param void
 *  @return
*/
void MqConnection::DoReconnWork()
{
    MQ_INFO("ReConnect mq thread start");
    m_bReConn = true;//可能在重连线程还未运行时又触发了Mq异常，所以此处和重连线程内部都设置标记
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
 *  @brief Mq连接异常回调
 *  @param [in]ex 异常消息 
 *  @return
*/
void MqConnection::onException(const CMSException& ex)
{
    MQ_ERROR("Mq connection exception, errmsg %s",ex.getMessage().c_str());
    ReConn();
}

/** @fn    ReBuildConn
 *  @brief 重新创建Mq连接，创建消费者和生产者
 *  @param  void
 *  @return true-成功，false-失败
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
 *  @brief 销毁Mq生产者、消费者，断开Mq连接
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
 *  @brief 发送消息
 *  @param [in]消息内容
 *  @return 成功返回0，否则返回其他值
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
 *  @brief 生产者创建
 *  @param [in]pSession Mq连接会话
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
 *  @brief 生产者销毁
 *  @param NULL
 *  @return 
*/
void MqProducer::Destroy()
{
    ReleaseDst(m_pDestination,m_pProducer);
    m_pSession = NULL;
}

/** @fn    Create
 *  @brief 消费者创建
 *  @param [in]pSession Mq连接会话
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
 *  @brief 消费者销毁
 *  @param NULL
 *  @return 
*/
void MqConsumer::Destroy()
{
    ReleaseDst(m_pDestination,m_pConsumer);
    m_pSession = NULL;
}

/** @fn    onMessage
 *  @brief Mq消息回调函数
 *  @param [in]message 回调的消息
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