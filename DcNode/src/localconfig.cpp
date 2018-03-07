#include "localconfig.h"


int LocalConfig::LoadXml(const char* xml)
{
	XMLDocument doc;  
    	if(doc.LoadFile(xml)!=0)     
    	{  
    	    DC_ERROR("load %s err",xml);  
    	    return SWARTZ_ERR;  
   	}     
  
   	XMLElement *root = doc.RootElement();  	
	XMLElement *host = root->FirstChildElement("host");    
	if(host == NULL)  
    	{  
    	    DC_ERROR("load %s err",xml);  
    	    return SWARTZ_ERR;  
    	}
	m_NodeInfo.NodeIp = host->Attribute("NodeIp");   
	m_NodeInfo.NodePort = host->IntAttribute("NodePort");   
	m_NodeInfo.ClusterIp = host->Attribute("ClusterIp");   
	m_NodeInfo.ClusterPort = host->IntAttribute("ClusterPort");  
	
	XMLElement *mq = root->FirstChildElement("mq");    
	if(mq == NULL)  
    	{  
    	    DC_ERROR("load %s err",xml);  
    	    return SWARTZ_ERR;  
    	}
	m_NodeInfo.mqIP = mq->Attribute("ip");   
	m_NodeInfo.mqPort = mq->IntAttribute("port");   
	m_NodeInfo.mqQueue = mq->Attribute("queue");   
	return SWARTZ_OK; 
}
