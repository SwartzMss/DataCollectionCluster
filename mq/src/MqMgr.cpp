#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include "libactivemq.h"
#include "MqMgr.h"

/** @fn MqMgr_Init
 *  @brief 初始化。
 *  @param void
 *  @return 成功返回0，否则返回其他值。
 */
int MqMgr::MqMgr_Init()
{
    activemq::library::ActiveMQCPP::initializeLibrary();
    MQ_INFO("Init mq module succ");
    return 0;
}

/** @fn MqMgr_Fini
 *  @brief 反初始化。
 *  @param void
 *  @return void
 */
void MqMgr::MqMgr_Fini()
{
    for (map<string,MqConnection*>::iterator itr = m_mapConn.begin(); itr != m_mapConn.end(); ++itr)
    {
        itr->second->DisConn();
        delete itr->second;
        itr->second = NULL;
    }

    m_mapProcucer.clear();
    m_mapConn.clear();

    activemq::library::ActiveMQCPP::shutdownLibrary();
    MQ_INFO("Fini mq module succ");
}

/** @fn AddConnection
 *  @brief 添加连接
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @return true-是，false-否
 */
MqConnection* MqMgr::AddConnection(const string& strMQSvr)
{
    MqConnection* pConn = NULL;
    map<string,MqConnection*>::iterator itr = m_mapConn.find(strMQSvr);
    if (itr != m_mapConn.end())
    {
        pConn = itr->second;
    }
    else
    {
        pConn = new(std::nothrow) MqConnection(strMQSvr);
        if (pConn)
        {
            MQ_INFO("Add connection %s",strMQSvr.c_str());
            pConn->Conn();
            m_mapConn.insert(map<string,MqConnection*>::value_type(strMQSvr,pConn));
        }
        else
        {
            MQ_ERROR("New memory fail");
        }
    }
    
    return pConn;
}

/** @fn MqMgr_AddConsumer
 *  @brief 添加MQ消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @param [in]pfnMsgCb MQ消息回调
   *  @param [in]pUser 用户指针
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr::MqMgr_AddConsumer(const string& strUrl,MqMsgCb pfnMsgCb,void* pUser)
{
    MQ_INFO("Add comsumer %s",strUrl.c_str());
    if (NULL == pfnMsgCb)
    {
        MQ_ERROR("Null mq message callback");
        return -1;
    }

    string strMqSvr;
    string strDst;
    MqMsgMode nMode = MQ_MODE_QUEUE;
    if (!ParseMqUri(strUrl,strMqSvr,strDst,nMode))
    {
        MQ_ERROR("Parse mq url fail");
        return -1;
    }

    if (HasConsumer(strMqSvr,strDst,nMode))
    {
        return 0;
    }


    MqConnection* pConn = AddConnection(strMqSvr);
    if (pConn && pConn->AddConsumer(strDst,nMode,pfnMsgCb,pUser))
    {
        return 0;
    }
    
    MQ_ERROR("Add consumer %s fail",strUrl.c_str());
    return -1;
}

/** @fn MqMgr_DelConsumer
 *  @brief 删除消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr::MqMgr_DelConsumer(const string& strUrl)
{
    MQ_INFO("delete consumer %s",strUrl.c_str());
    string strMqSvr;
    string strDst;
    MqMsgMode nMode = MQ_MODE_QUEUE;
    if (!ParseMqUri(strUrl,strMqSvr,strDst,nMode))
    {
        MQ_ERROR("Parse mq url %s fail",strUrl.c_str());
        return -1;
    }

    map<string,MqConnection*>::iterator itr = m_mapConn.find(strMqSvr);
    if (itr != m_mapConn.end())
    {
        bool bEmptyConn = itr->second->DelConsumer(strDst,nMode);
        if (bEmptyConn)//如果连接是没有消费者或者生产者的空连接，删除
        {
            itr->second->DisConn();
            delete itr->second;
            m_mapConn.erase(itr);
        }
    }

    MQ_INFO("delete consumer %s succ",strUrl.c_str());
    return 0;
}

/** @fn MqMgr_AddProducer
 *  @brief 添加MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 非空-生产者指针，空-失败
 */
MqProducer* MqMgr::MqMgr_AddProducer(const string& strUrl)
{
    MQ_INFO("Add producer %s",strUrl.c_str());
    string strMqSvr;
    string strDst;
    MqMsgMode nMode = MQ_MODE_QUEUE;
    if (!ParseMqUri(strUrl,strMqSvr,strDst,nMode))
    {
        MQ_ERROR("Parse mq url %s fail",strUrl.c_str());
        return NULL;
    }

    if (HasProducer(strUrl))
    {
        return m_mapProcucer[strUrl];
    }

    MqConnection* pConn = AddConnection(strMqSvr);
    if (NULL == pConn)
    {
        MQ_ERROR("Add conn %s fail",strMqSvr.c_str());
        return NULL;
    }

    MqProducer* pPro = pConn->AddProducer(strDst,nMode);
    if (pPro)
    {
        //此处缓存生产者为了提高发送时的查找效率
        m_mapProcucer.insert(map<string,MqProducer*>::value_type(strUrl,pPro));
        MQ_INFO("Add producer %s succ",strUrl.c_str());
    }

    return pPro;
}

/** @fn MqMgr_DelProducer
 *  @brief 删除MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr::MqMgr_DelProducer(const string& strUrl)
{
    string strMqSvr;
    string strDst;
    MqMsgMode nMode = MQ_MODE_QUEUE;
    if (!ParseMqUri(strUrl,strMqSvr,strDst,nMode))
    {
        MQ_ERROR("Parse mq url fail");
        return -1;
    }

    map<string,MqConnection*>::iterator itr = m_mapConn.find(strMqSvr);//erase from conn
    if (itr != m_mapConn.end())
    {
        bool bEmptyConn = itr->second->DelProducer(strDst,nMode);
        if (bEmptyConn)//如果连接是没有消费者或者生产者的空连接，删除
        {
            itr->second->DisConn();
            delete itr->second;
            m_mapConn.erase(itr);
        }
    }

    map<string,MqProducer*>::iterator itrPro = m_mapProcucer.find(strUrl);//erase from cache
    if (itrPro != m_mapProcucer.end())
    {
        m_mapProcucer.erase(itrPro);
    }

    MQ_INFO("delete producer %s succ",strUrl.c_str());
    return 0;
}

/** @fn MqMgr_SendMsg
 *  @brief 发送消息
 *  @param [in]strUrl  发送目的MQ  url
 *  @param [in]strMsg  消息内容
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr::MqMgr_SendMsg(const string& strUrl,const string& strMsg)
{
    {
        map<string,MqProducer*>::iterator itr = m_mapProcucer.find(strUrl);
        if (itr != m_mapProcucer.end())
        {
            return itr->second->SendMsg(strMsg);
        }
    }
    
    {
        MQ_INFO("Send msg dst %s not exist",strUrl.c_str());
        MqProducer* pPro = MqMgr_AddProducer(strUrl);//加锁不能放到此行前面，因为AddProducer需要锁
        if (NULL == pPro)
        {
            return -1;
        }
        MQ_INFO("Add msg dst %s succ",strUrl.c_str());

        return pPro->SendMsg(strMsg);
    }
}

/** @fn ParseMqUri
 *  @brief 解析Mq uri
 *  @param [in]strMqUri   Mq uri，绝对路径，格式形如mq/127.0.0.1:61618/queue/acs.event.queue
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @param [in]strDstName Queue或者Queue名称
 *  @param [in]nType      Mq连接类型
 *  @return true-成功，false-失败
 */
bool MqMgr::ParseMqUri(const string& strMqUri, string& strMQSvr, string& strDstName, MqMsgMode& nType)
{
    do 
    {
        int nCurr = 0;
        int nIndex = strMqUri.find("mq/");
        if (nIndex == strMqUri.npos)
        {
            MQ_ERROR("Parse mq uri %s fail, uri without mq",strMqUri.c_str());
            break;
        }
        nCurr = nIndex + 3;

        nIndex = strMqUri.find('/',nCurr);//mq server
        if (nIndex == strMqUri.npos)
        {
            MQ_ERROR("Parse mq uri %s fail, no mq server",strMqUri.c_str());
            break;
        }
        strMQSvr = strMqUri.substr(nCurr,nIndex - nCurr);
        nCurr = nIndex + 1;

        nIndex = strMqUri.find('/',nCurr);//queue or topic
        if (nIndex == strMqUri.npos)
        {
            MQ_ERROR("Parse mq uri %s fail, no mq message mode",strMqUri.c_str());
            break;
        }
        string strType = strMqUri.substr(nCurr, nIndex - nCurr);
        if (strType == "queue")
        {
            nType = MQ_MODE_QUEUE;
        }
        else if(strType == "topic")
        {
            nType = MQ_MODE_TOPIC;
        }
        else
        {
            MQ_ERROR("Unkown mq mode");
            break;
        }
        nCurr = nIndex + 1;

        strDstName = strMqUri.substr(nCurr);
        if (strDstName.empty())
        {
            MQ_ERROR("Parse mq uri %s fail, no mq dst",strMqUri.c_str());
            break;
        }

        return true;
    } while (false);

    return false;
}

/** @fn HasConsumer
 *  @brief 查找消费者是否存在
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @param [in]strDstName Queue或者Queue名称
 *  @param [in]nType      Mq连接类型
 *  @return true-是，false-否
 */
bool MqMgr::HasConsumer(const string& strMQSvr, string strDstName, MqMsgMode nType)
{
    map<string,MqConnection*>::iterator itrCon = m_mapConn.find(strMQSvr);
    if (itrCon != m_mapConn.end())
    {
        return itrCon->second->HasConsumer(strDstName,nType);
    }

    return false;
}

/** @fn HasProducer
 *  @brief 查找生产者是否存在
 *  @param [in]strMqUri   Mq uri，绝对路径，格式形如mq/127.0.0.1:61618/queue/acs.event.queue
 *  @return true-是，false-否
 */
bool MqMgr::HasProducer(const string& strMqUri)
{
    map<string,MqProducer*>::iterator itr = m_mapProcucer.find(strMqUri);
    return (itr != m_mapProcucer.end());
}
