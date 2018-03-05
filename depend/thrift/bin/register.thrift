include "common.thrift"


service Regist {
    common.RegistResult registClient(1:common.ClientInfo clientInfo),
    bool heartbeat(1:common.HeartBeatInfo heartbeatInfo)
}
