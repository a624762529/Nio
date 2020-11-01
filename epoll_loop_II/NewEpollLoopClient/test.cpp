//#include<iostream>
//#include"sockclient.h"
//#include<atomic>
//#include"infohead.h"


//using namespace std;


////测试整段报文发送
//int main123(int argc, char *argv[])
//{
//    SockClient client("127.0.0.1",8888);
//    client.connectToHost();

//    cout<<"连接成功"<<endl;

//    {
//        InfoPack *pack=reinterpret_cast<InfoPack*>(new char[102 + 4]);
//        pack->len=102;
//        for(int i=0;i<102;i++)
//        {
//            pack->data[i]='1';
//        }
//        client.writeInfo(reinterpret_cast<char*>(pack),106);
//        free(pack);
//    }

//    {
//        InfoPack *pack=reinterpret_cast<InfoPack*>(new char[1024 + 4]);
//        int  ret=client.readInfo(reinterpret_cast<char*>(pack),1024+4);
//        if(ret>0)
//        {
//            cout<<"信息长度"<<pack->len<<endl;
//            cout<<"信息内容"<<pack->data<<endl;
//            cout<<"strlen info:"<<strlen(pack->data)<<endl;
//        }
//        free(pack);
//    }
//    return 1;
//}

//bool         spy=true;
//atomic<int>  times;
//#define timesaccess 10
//void test()
//{
//    sleep(timesaccess);
//    spy=false;
//}
////测试一段报文分两次  甚至3次发送和接收


///*
//    可以做到心跳断开

//    不过....
//*/
//void testSec()
//{
//    int loop_times=0;
//    SockClient client("127.0.0.1",8888);
//    client.connectToHost();
//    cout<<"连接成功"<<endl;
//    while (spy)
//    {
//        loop_times++;
//        {
//            InfoPack *pack=reinterpret_cast<InfoPack*>(new char[102 + 4]);
//            pack->len=102;
//            for(int i=0;i<102;i++)
//            {
//                pack->data[i]='1';
//            }
//            int read_len=0;
//            while (true)
//            {
//                int ret=client.writeInfo(reinterpret_cast<char*>(pack),106);
//                read_len+=ret;
//                if(read_len>=106)
//                {
//                    break;
//                }
//            }
//            free(pack);
//        }
//        {
//            InfoPack *pack=reinterpret_cast<InfoPack*>(new char[1024 + 4]);
//            client.stableRecv(reinterpret_cast<char*>(pack),4);
//            client.stableRecv(pack->data,pack->len);
//            free(pack);
//        }
//        times++;
//    }

//    cout<<"loop times"<<loop_times<<endl;
//}

//#define MaxConnectNum 1
//int main123123123123()
//{
//    signal(SIGPIPE,SIG_IGN);
//    times=0;
//    thread th(test);
//    th.detach();

//    SockClient client[102];
//    for(int i=0;i<100;i++)
//    {
//        client[i].createSocket("127.0.0.1",8888);
//        client[i].connectToHost();
//    }


//    thread task[MaxConnectNum];
//    for(int i=0;i<MaxConnectNum;i++)
//    {
//        task[i]=std::move(thread(testSec));
//    }

//    for(int i=0;i<MaxConnectNum;i++)
//    {
//        task[i].join();
//    }

//    cout<<"极限压力测试的次数"<<times/timesaccess<<endl;

//    return 1;
//}


//int main000()
//{
//    SockClient client[102];
//    for(int i=0;i<102;i++)
//    {
//        client[i].createSocket("127.0.0.1",8888);
//        client[i].connectToHost();
//    }
//    pause();
//    return 1;
//}


//int mai235465n()
//{
//    SockClient client[102];
//    for(int i=0;i<1;i++)
//    {
//        client[i].createSocket("127.0.0.1",8888);
//        client[i].connectToHost();
//    }


//    return 1;
//}
//#include<map>

//int main2314()
//{
//    map<int*,int> mp;
//    mp.insert(map<int*,int>::value_type( (int *)-1,3 ));
//    if( mp.find( (int*)-1 )==mp.end() )
//    {
//        cout<<"find value"<<endl;
//    }

//    return 1;
//}
