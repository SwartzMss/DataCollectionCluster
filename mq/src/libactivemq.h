#ifndef _COMM_MGR_H_
#define _COMM_MGR_H_

#define BEDS_EXTERN extern "C"
#define BEDS_API


#include <string>

/** @fn CommMgr_Init
*  @brief ��ʼ����
*  @param void
*  @return �ɹ�����0�����򷵻�����ֵ��
*/
BEDS_EXTERN int BEDS_API CommMgr_Init();

/** @fn CommMgr_Fini
*  @brief ����ʼ����
*  @param void
*  @return void
*/
BEDS_EXTERN void BEDS_API CommMgr_Fini();

/** @fn MqMsgCb
*  @brief MQ��������Ϣ�ص�������
*  @param [in]pMsg  ��Ϣ����
*  @param [in]uMsgLen ��Ϣ����
*  @param [in]pUser �û�ָ��
*  @return null
*/
typedef void (BEDS_API* MqMsgCb)(const char* pMsg, unsigned uMsgLen,void* pUser);

/** @fn CommMgr_AddConsumer
*  @brief ���MQ������
*  @param [in]strUrl  ������MQ  url
*  @param [in]pfnMsgCb MQ��Ϣ�ص�
*  @param [in]pUser �û�ָ��
*  @return �ɹ�����0�����򷵻�����ֵ
*/
BEDS_EXTERN int BEDS_API CommMgr_AddConsumer(const std::string& strUrl,MqMsgCb pfnMsgCb,void* pUser);

/** @fn CommMgr_DelConsumer
*  @brief ɾ��������
*  @param [in]strUrl  ������MQ  url
*  @return �ɹ�����0�����򷵻�����ֵ
*/
BEDS_EXTERN int BEDS_API CommMgr_DelConsumer(const std::string& strUr);

/** @fn CommMgr_AddProducer
*  @brief ���MQ�����ߣ�Ԥ��
*  @param [in]strUrl  MQ  url
*  @return �ɹ�����0�����򷵻�����ֵ
*/
BEDS_EXTERN int BEDS_API CommMgr_AddProducer(const std::string& strUrl);

/** @fn CommMgr_DelProducer
*  @brief ɾ��MQ�����ߣ�Ԥ��
*  @param [in]strUrl  MQ  url
*  @return �ɹ�����0�����򷵻�����ֵ
*/
BEDS_EXTERN int BEDS_API CommMgr_DelProducer(const std::string& strUrl);

/** @fn CommMgr_SendMsg
*  @brief ������Ϣ
*  @param [in]strUrl  ����Ŀ��MQ  url
*  @param [in]strMsg  ��Ϣ����
*  @return �ɹ�����0�����򷵻�����ֵ
*/
BEDS_EXTERN int BEDS_API CommMgr_SendMsg(const std::string& strUrl,const std::string& strMsg);

#endif //_COMM_MGR_H_
