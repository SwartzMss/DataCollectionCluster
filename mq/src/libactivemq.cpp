#include "libactivemq.h"
#include "MqMgr.h"


/** @fn CommMgr_Init
 *  @brief 初始化。
 *  @param void
 *  @return 成功返回0，否则返回其他值。
 */
BEDS_EXTERN int BEDS_API CommMgr_Init()
{
    if (0 == MqMgr::Instance()->MqMgr_Init())
    {
        return 0;
    }

    CommMgr_Fini();
    return -1;
    
}

/** @fn CommMgr_Fini
 *  @brief 反初始化。
 *  @param void
 *  @return void
 */
BEDS_EXTERN void BEDS_API CommMgr_Fini()
{
    MqMgr::Instance()->MqMgr_Fini();
}


/** @fn CommMgr_AddConsumer
 *  @brief 添加MQ消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @param [in]pfnMsgCb MQ消息回调
 *  @param [in]pUser 用户指针
 *  @return 成功返回0，否则返回其他值
 */
BEDS_EXTERN int BEDS_API CommMgr_AddConsumer(const string& strUrl,MqMsgCb pfnMsgCb,void* pUser)
{
    return MqMgr::Instance()->MqMgr_AddConsumer(strUrl,pfnMsgCb,pUser);
}

/** @fn CommMgr_DelConsumer
 *  @brief 删除消费者
 *  @param [in]strUrl  消费者MQ  url
 *  @return 成功返回0，否则返回其他值
 */
BEDS_EXTERN int BEDS_API CommMgr_DelConsumer(const string& strUr)
{
    return MqMgr::Instance()->MqMgr_DelConsumer(strUr);
}

/** @fn CommMgr_AddProducer
 *  @brief 添加MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 成功返回0，否则返回其他值
 */
BEDS_EXTERN int BEDS_API CommMgr_AddProducer(const string& strUrl)
{
    return (MqMgr::Instance()->MqMgr_AddProducer(strUrl) != NULL ? 0:-1);
}

/** @fn CommMgr_DelProducer
 *  @brief 删除MQ生产者，预留
 *  @param [in]strUrl  MQ  url
 *  @return 成功返回0，否则返回其他值
 */
BEDS_EXTERN int BEDS_API CommMgr_DelProducer(const string& strUrl)
{
    return MqMgr::Instance()->MqMgr_DelProducer(strUrl);
}

/** @fn CommMgr_SendMsg
 *  @brief 发送消息
 *  @param [in]strUrl  发送目的MQ  url
 *  @param [in]strMsg  消息内容
 *  @return 成功返回0，否则返回其他值
 */
BEDS_EXTERN int BEDS_API CommMgr_SendMsg(const string& strUrl,const string& strMsg)
{
    return MqMgr::Instance()->MqMgr_SendMsg(strUrl,strMsg);
}

