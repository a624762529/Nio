#ifndef INFOHEAD_H
#define INFOHEAD_H

struct InfoPack
{
    int  info_len;
    char m_data;
};
struct TalkInfo
{
    enum
    {
        JoinHome,SendInfo,SendSuccess,SendFail
    };
    int  m_info_len; //信息的长度
    int  m_home_id;  //信息的id
    int  m_type;
    char m_info;     //信息的内容
};

#endif // INFOHEAD_H
