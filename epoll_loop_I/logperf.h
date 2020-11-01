#ifndef LOGPERF_H
#define LOGPERF_H
#include"head.h"
#include"sem_pv.h"
using namespace std;
/*
    高性能日志设计思路
        维护四块内存 (4096*3) *4
        要是其中一个被填满 那就使用它写到磁盘中去
        日志的存放格式
            线程id的后四位   日期     核心信息（例如 用户名 指令 文件）
        每个文件最多只能放50M以内的数据 超过了 创建另一个临时文件
                                        （例如 path1 path2..等等）


    设计思路使用多线程技术中的生产者与消费者逻辑
    如果其中一个被写满 那就通知其子线程  让它去讲其中内容写入到文件中

    日志的优化地方
        减少的磁盘Io操作加速程序的运行效率
*/


#define log_magic 11
#define max_info 4096*3

class Logperf
{
public:
    Logperf()=delete;
    Logperf(string path,int flag);
    ~Logperf();
    void Put_into_log(string info);
    void set_can_used();
    void set_cannot_used();
    void start_write();
private:
    void change_file();//如果对应的文件所占据的空间已经大于50M
                       //那就重新创建一个文件
    int  get_file_size();
    void init_Log_Info(string path,int flag);
    void gettime(char *cur_time);
                                //消费者 消费空闲资源 讲空闲资源写入到文件里面
    void save_requir();         //通知消费者让他消费
    void talk_to_commer();
    void close_log();
private:
    int magic;                          //启动数据 要是失败 那就抛出异常
    string ch_arry[4];                  //数据存放区域
    atomic<int> free_index;             //空心区块的索引
    atomic<int> write_to_file_index;    //需要写在文件中的索引
    //int next_free_index;              //下一个空闲块的索引
    int max;

private:
    mutex file_mu;
    int fd;                     //log 文件描述符
    string path;                //log 路径
    string default_path;        //log 默认的文件路径 参数给的路径打开失败 才存放到默认的路径里面
    string file_name;
    atomic<int> index;          //创建的第n个文件索引
private:
    mutex mu_log;
private:
    Sem_Pv free_qua;
    Sem_Pv nofree_qua;
    mutex  mu;
public:
    atomic<int> record_write_file;
};

#endif // LOGPERF_H
