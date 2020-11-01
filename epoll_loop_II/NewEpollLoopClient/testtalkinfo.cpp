#include<iostream>
#include"sockclient.h"
#include<atomic>
#include"infohead.h"
using namespace std;

InfoPack* createJoinHomeInfo()
{
    InfoPack *send_pack=(InfoPack *)
            malloc(sizeof(InfoPack)+sizeof(TalkInfo));
    memset(send_pack,0,sizeof(InfoPack)+sizeof(TalkInfo));
    send_pack->info_len= sizeof(TalkInfo);
    TalkInfo * join_home   =(TalkInfo *)(&send_pack->m_data);
    join_home->m_home_id  =100;
    join_home->m_info_len =sizeof(TalkInfo);
    join_home->m_type=TalkInfo::JoinHome;
    join_home->m_info=0;
    return send_pack;
}

InfoPack* createTalkInfo(char *info)
{
    InfoPack *send_pack=(InfoPack *)
            malloc(sizeof(InfoPack)+
                   sizeof(TalkInfo)+
                   strlen(info));
    memset(send_pack,0,sizeof(InfoPack)+
           sizeof(TalkInfo)+strlen(info));

    send_pack->info_len= sizeof(TalkInfo)+strlen(info);
    TalkInfo * sendinfo   =(TalkInfo *)(&send_pack->m_data);
    sendinfo->m_home_id  =100;
    sendinfo->m_info_len =sizeof(TalkInfo)+strlen(info);
    sendinfo->m_type=TalkInfo::SendInfo;
    sendinfo->m_info=strlen(info);
    memcpy(&sendinfo->m_info,info,strlen(info));

    return send_pack;
}

int main()
{



    SockClient cli;
    cli.createSocket("127.0.0.1",8888);
    cli.connectToHost();

    SockClient cli_per;
    cli_per.createSocket("127.0.0.1",8888);
    cli_per.connectToHost();

    InfoPack *send_pack=createJoinHomeInfo();
    cli.writeInfo
            (reinterpret_cast<char*>(send_pack),
              send_pack->info_len+4);
    cli_per.writeInfo
            (reinterpret_cast<char*>(send_pack),
             send_pack->info_len+4);


//    char read_buf[1024]{};
//    InfoPack * pack=createTalkInfo("hellow");



//    cli.writeInfo((char*)pack,pack->info_len+4);
//    cli_per.readInfo(read_buf,sizeof(read_buf));
//    InfoPack* info=reinterpret_cast< InfoPack*>(read_buf);
//    TalkInfo* real_info=reinterpret_cast< TalkInfo*>
//            (&info->m_data);
//    cout<<&real_info->m_info<<endl;


//    cli_per.writeInfo((char*)pack,pack->info_len+4);
//    cli.readInfo(read_buf,sizeof(read_buf));

//    InfoPack* infov=reinterpret_cast< InfoPack*>(read_buf);
//    TalkInfo* real_infov=reinterpret_cast< TalkInfo*>
//            (&infov->m_data);
//    cout<<&real_infov->m_info<<endl;


    return 1;
}
