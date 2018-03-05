namespace cpp  DcCluster


enum RegistResult {
    SUCCESS,
    INVALIE_PARA,
    UNDEFINE_ERR,	
}

struct ClientInfo {
    1: string Ip,
    2: i32 Port,
    3: optional string msg,
}

struct HeartBeatInfo {
    1: string Ip,
    2: i32 Port,
    3: optional string msg,
}
