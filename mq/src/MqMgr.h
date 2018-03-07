#ifndef _MQ_MGR_H_
#define _MQ_MGR_H_


#include "MqConnection.h"


class MqMgr 
{
public:
    static MqMgr* Instance(){static MqMgr singleton; return &singleton;}


/** @fn MqMgr_Init
 *  @brief ��ʼ����
 *  @param void
 *  @return �ɹ�����0�����򷵻�����ֵ��
 */
int MqMgr_Init();


/** @fn MqMgr_Fini
 *  @brief ����ʼ����
 *  @param void
 *  @return void
 */
void MqMgr_Fini();


/** @fn MqMgr_AddConsumer
 *  @brief ���MQ������
 *  @param [in]strUrl  ������MQ  url
 *  @param [in]pfnMsgCb MQ��Ϣ�ص�
 *  @param [in]pUser �û�ָ��
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
int MqMgr_AddConsumer(const string& strUrl,MqMsgCb pfnMsgCb,void* pUser);


/** @fn MqMgr_DelConsumer
 *  @brief ɾ��������
 *  @param [in]strUrl  ������MQ  url
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
int MqMgr_DelConsumer(const string& strUr);


/** @fn MqMgr_AddProducer
 *  @brief ���MQ�����ߣ�Ԥ��
 *  @param [in]strUrl  MQ  url
 *  @return �ǿ�-������ָ�룬��-ʧ��
 */
MqProducer* MqMgr_AddProducer(const string& strUrl);


/** @fn MqMgr_DelProducer
 *  @brief ɾ��MQ�����ߣ�Ԥ��
 *  @param [in]strUrl  MQ  url
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
int MqMgr_DelProducer(const string& strUrl);


/** @fn MqMgr_SendMsg
 *  @brief ������Ϣ
 *  @param [in]strUrl  ����Ŀ��MQ  url
 *  @param [in]strMsg  ��Ϣ����
 *  @return �ɹ�����0�����򷵻�����ֵ
 */
int MqMgr_SendMsg(const string& strUrl,const string& strMsg);


private:
    map<string,MqConnection*> m_mapConn;//<MqIp:Port,Conn>ͨ��ģ��Mq����
    map<string,MqProducer*> m_mapProcucer;//<ProducerURI,Producer>ͨ��ģ��Mq����������


/** @fn ParseMqUri
 *  @brief ����Mq uri
 *  @param [in]strMqUri   Mq uri������·������ʽ����mq/127.0.0.1:61618/queue/acs.event.queue
 *  @param [in]strMQSvr   Mq �����ַ����ʽ127.0.0.1:80
 *  @param [in]strDstName Queue����Queue����
 *  @param [in]nType      Mq��������
 *  @return true-�ɹ���false-ʧ��
 */
    bool ParseMqUri(const string& strMqUri, string& strMQSvr, string& strDstName, MqMsgMode& nType);


/** @fn HasConsumer
 *  @brief �����������Ƿ����
 *  @param [in]strMQSvr   Mq �����ַ����ʽ127.0.0.1:80
 *  @param [in]strDstName Queue����Queue����
 *  @param [in]nType      Mq��������
 *  @return true-�ǣ�false-��
 */
    bool HasConsumer(const string& strMQSvr, string strDstName, MqMsgMode nType);


/** @fn HasProducer
 *  @brief �����������Ƿ����
 *  @param [in]strMqUri   Mq uri������·������ʽ����mq/127.0.0.1:61618/queue/acs.event.queue
 *  @return true-�ǣ�false-��
 */
    bool HasProducer(const string& strMqUri);


/** @fn AddConnection
 *  @brief �������
 *  @param [in]strMQSvr   Mq �����ַ����ʽ127.0.0.1:80
 *  @return true-�ǣ�false-��
 */
    MqConnection* AddConnection(const string& strMQSvr);
};


#endif