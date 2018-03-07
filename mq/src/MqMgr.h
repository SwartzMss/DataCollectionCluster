#ifndef _MQ_MGR_H_
#define _MQ_MGR_H_


#include "MqConnection.h"


class MqMgr 
{
public:
    static MqMgr* Instance(){static MqMgr singleton; return &singleton;}


/** @fn MqMgr_Init
 *  @brief 初始化。
 *  @param void
 *  @return 成功返回0，否则返回其他值。
 */
int MqMgr_Init();


/** @fn MqMgr_Fini
 *  @brief 反初始化。
 *  @param void
 *  @return void
 */
void MqMgr_Fini();


/** @fn MqMgr_AddConsumer
 *  @brief 添加MQ消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @param [in]pfnMsgCb MQ消息回调
 *  @param [in]pUser 用户指针
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr_AddConsumer(const string& strUrl,MqMsgCb pfnMsgCb,void* pUser);


/** @fn MqMgr_DelConsumer
 *  @brief 删除消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr_DelConsumer(const string& strUr);


/** @fn MqMgr_AddProducer
 *  @brief 添加MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 非空-生产者指针，空-失败
 */
MqProducer* MqMgr_AddProducer(const string& strUrl);


/** @fn MqMgr_DelProducer
 *  @brief 删除MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr_DelProducer(const string& strUrl);


/** @fn MqMgr_SendMsg
 *  @brief 发送消息
 *  @param [in]strUrl  发送目的MQ  url
 *  @param [in]strMsg  消息内容
 *  @return 成功返回0，否则返回其他值
 */
int MqMgr_SendMsg(const string& strUrl,const string& strMsg);


private:
    map<string,MqConnection*> m_mapConn;//<MqIp:Port,Conn>通信模块Mq连接
    map<string,MqProducer*> m_mapProcucer;//<ProducerURI,Producer>通信模块Mq所有生产者


/** @fn ParseMqUri
 *  @brief 解析Mq uri
 *  @param [in]strMqUri   Mq uri，绝对路径，格式形如mq/127.0.0.1:61618/queue/acs.event.queue
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @param [in]strDstName Queue或者Queue名称
 *  @param [in]nType      Mq连接类型
 *  @return true-成功，false-失败
 */
    bool ParseMqUri(const string& strMqUri, string& strMQSvr, string& strDstName, MqMsgMode& nType);


/** @fn HasConsumer
 *  @brief 查找消费者是否存在
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @param [in]strDstName Queue或者Queue名称
 *  @param [in]nType      Mq连接类型
 *  @return true-是，false-否
 */
    bool HasConsumer(const string& strMQSvr, string strDstName, MqMsgMode nType);


/** @fn HasProducer
 *  @brief 查找生产者是否存在
 *  @param [in]strMqUri   Mq uri，绝对路径，格式形如mq/127.0.0.1:61618/queue/acs.event.queue
 *  @return true-是，false-否
 */
    bool HasProducer(const string& strMqUri);


/** @fn AddConnection
 *  @brief 添加连接
 *  @param [in]strMQSvr   Mq 服务地址，格式127.0.0.1:80
 *  @return true-是，false-否
 */
    MqConnection* AddConnection(const string& strMQSvr);
};


#endif