#ifndef _COMM_MGR_H_
#define _COMM_MGR_H_

#define BEDS_EXTERN extern "C"
#define BEDS_API


#include <string>

/** @fn CommMgr_Init
*  @brief 初始化。
*  @param void
*  @return 成功返回0，否则返回其他值。
*/
BEDS_EXTERN int BEDS_API CommMgr_Init();

/** @fn CommMgr_Fini
*  @brief 反初始化。
*  @param void
*  @return void
*/
BEDS_EXTERN void BEDS_API CommMgr_Fini();

/** @fn MqMsgCb
*  @brief MQ消费者消息回调函数。
*  @param [in]pMsg  消息内容
*  @param [in]uMsgLen 消息长度
*  @param [in]pUser 用户指针
*  @return null
*/
typedef void (BEDS_API* MqMsgCb)(const char* pMsg, unsigned uMsgLen,void* pUser);

/** @fn CommMgr_AddConsumer
*  @brief 添加MQ消费者
*  @param [in]strUrl  消费者MQ  url
*  @param [in]pfnMsgCb MQ消息回调
*  @param [in]pUser 用户指针
*  @return 成功返回0，否则返回其他值
*/
BEDS_EXTERN int BEDS_API CommMgr_AddConsumer(const std::string& strUrl,MqMsgCb pfnMsgCb,void* pUser);

/** @fn CommMgr_DelConsumer
*  @brief 删除消费者
*  @param [in]strUrl  消费者MQ  url
*  @return 成功返回0，否则返回其他值
*/
BEDS_EXTERN int BEDS_API CommMgr_DelConsumer(const std::string& strUr);

/** @fn CommMgr_AddProducer
*  @brief 添加MQ生产者，预留
*  @param [in]strUrl  MQ  url
*  @return 成功返回0，否则返回其他值
*/
BEDS_EXTERN int BEDS_API CommMgr_AddProducer(const std::string& strUrl);

/** @fn CommMgr_DelProducer
*  @brief 删除MQ生产者，预留
*  @param [in]strUrl  MQ  url
*  @return 成功返回0，否则返回其他值
*/
BEDS_EXTERN int BEDS_API CommMgr_DelProducer(const std::string& strUrl);

/** @fn CommMgr_SendMsg
*  @brief 发送消息
*  @param [in]strUrl  发送目的MQ  url
*  @param [in]strMsg  消息内容
*  @return 成功返回0，否则返回其他值
*/
BEDS_EXTERN int BEDS_API CommMgr_SendMsg(const std::string& strUrl,const std::string& strMsg);

#endif //_COMM_MGR_H_
