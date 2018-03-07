#include "libactivemq.h"
#include "MqMgr.h"


/** @fn CommMgr_Init
 *  @brief ��ʼ����
 *  @param void
 *  @return �ɹ�����0�����򷵻�����ֵ��
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
 *  @brief ����ʼ����
 *  @param void
 *  @return void
 */
BEDS_EXTERN void BEDS_API CommMgr_Fini()
{
    MqMgr::Instance()->MqMgr_Fini();
}


/** @fn CommMgr_AddConsumer
 *  @brief ���MQ������
 *  @param [in]strUrl  ������MQ  url
 *  @param [in]pfnMsgCb MQ��Ϣ�ص�
 *  @param [in]pUser �û�ָ��
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
BEDS_EXTERN int BEDS_API CommMgr_AddConsumer(const string& strUrl,MqMsgCb pfnMsgCb,void* pUser)
{
    return MqMgr::Instance()->MqMgr_AddConsumer(strUrl,pfnMsgCb,pUser);
}

/** @fn CommMgr_DelConsumer
 *  @brief ɾ��������
 *  @param [in]strUrl  ������MQ  url
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
BEDS_EXTERN int BEDS_API CommMgr_DelConsumer(const string& strUr)
{
    return MqMgr::Instance()->MqMgr_DelConsumer(strUr);
}

/** @fn CommMgr_AddProducer
 *  @brief ���MQ�����ߣ�Ԥ��
 *  @param [in]strUrl  MQ  url
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
BEDS_EXTERN int BEDS_API CommMgr_AddProducer(const string& strUrl)
{
    return (MqMgr::Instance()->MqMgr_AddProducer(strUrl) != NULL ? 0:-1);
}

/** @fn CommMgr_DelProducer
 *  @brief ɾ��MQ�����ߣ�Ԥ��
 *  @param [in]strUrl  MQ  url
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
BEDS_EXTERN int BEDS_API CommMgr_DelProducer(const string& strUrl)
{
    return MqMgr::Instance()->MqMgr_DelProducer(strUrl);
}

/** @fn CommMgr_SendMsg
 *  @brief ������Ϣ
 *  @param [in]strUrl  ����Ŀ��MQ  url
 *  @param [in]strMsg  ��Ϣ����
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
BEDS_EXTERN int BEDS_API CommMgr_SendMsg(const string& strUrl,const string& strMsg)
{
    return MqMgr::Instance()->MqMgr_SendMsg(strUrl,strMsg);
}

