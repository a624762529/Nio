#include "logperf.h"
#include <cstring>
//Logperf log;

Logperf::Logperf(string path,int flag)
{
    init_Log_Info(path,flag);
    //初始化完成之后 启动消费者现场 试着让她们并发运行

}

void Logperf::start_write()
{
    if(magic!=MagicNum)
    {
        cout<<"魔数未初始化"<<__LINE__<<__FUNCTION__<<endl;
        throw bad_alloc();
    }
    thread th(&Logperf::save_requir,this);
    th.detach();
}

void Logperf::init_Log_Info(string path,int flag)
{
    this->index =0;
    file_name =path;
    default_path ="./log";
    record_write_file=0;
    char *cur_time = (char *)malloc(21*sizeof(char));
    gettime(cur_time);

    string path_=path+cur_time+" "+to_string(index);
    string path_tem= default_path+cur_time+" "+to_string(index);
    free(cur_time);
    cur_time=NULL;

    //初始化路径信息

    this->path         =path_;
    if(flag==O_EXCL)
        fd=open(path_.c_str(),O_RDWR|O_CLOEXEC,0777);
    else
        fd=open(path_.c_str(),O_RDWR|O_CREAT,0777);
    if(fd==-1)
    {
        //文件不存在 那就存放到默认的区域
        fd=open(path_tem.c_str(),O_RDWR|O_CREAT,0777);
        if(fd==-1)
        {
            cerr<<"log文件打开失败"<<__LINE__<<__FUNCTION__<<endl;
            throw bad_alloc();
        }
        this->path         =path_tem;
    }
    //

    //初始化索引信息
    max=max_info;
    free_index =0;          //初始化第一个空闲索引
    write_to_file_index=0;  //初始化要存入文件的索引
    //next_free_index =1;
    record_write_file=0;
    //初始化同步控制信息
    free_qua.set_CanUse();
    nofree_qua.set_CanUse();
    free_qua.set_Val(4);
    nofree_qua.set_Val(0);
}

void Logperf::gettime(char *cur_time)
{
    char Year[6] = {0};
    char Month[4] = {0};
    char Day[4] = {0};
    char Hour[4] = {0};
    char Min[4] = {0};
    char Sec[4] = {0};

    time_t current_time;
    struct tm* now_time;
    time(&current_time);
    now_time = localtime(&current_time);

    strftime(Year, sizeof(Year), "%Y-", now_time);
    strftime(Month, sizeof(Month), "%m-", now_time);
    strftime(Day, sizeof(Day), "%d ", now_time);
    strftime(Hour, sizeof(Hour), "%H:", now_time);
    strftime(Min, sizeof(Min), "%M:", now_time);
    strftime(Sec, sizeof(Sec), "%S", now_time);

    strncat(cur_time, Year, 5);
    strncat(cur_time, Month, 3);
    strncat(cur_time, Day, 3);
    strncat(cur_time, Hour, 3);
    strncat(cur_time, Min, 3);
    strncat(cur_time, Sec, 3);
}

void Logperf::Put_into_log(string info)
{
    if(magic!=MagicNum)
    {
        cout<<"魔数未初始化"<<endl;
        return;
    }
    if(info.size()==0)
    {
        return ;
    }

    {
        lock_guard<mutex> lock(mu_log);

        //讲参数中的信息存放到高速缓冲队列
        int file_size=get_file_size();
        if(file_size>50)
        {
            //如果文件大小大于10M 那就重新更换文件
            change_file();
        }
        ch_arry[free_index]+=info;
        if(ch_arry[free_index].size()>max_info)
        {
            talk_to_commer();
        }
    }
    return;
}

void Logperf::talk_to_commer()
{
    if(magic!=MagicNum)
    {
        cout<<"魔数未初始化"<<endl;
        return;
    }
    this->free_qua.sem_V();
    {
        lock_guard<mutex> lock(mu);
        free_index=(free_index+1)%4;//移动下一个索引
    }
    this->nofree_qua.sem_P();

}

void Logperf::save_requir()
{
    while (true)
    {
        if(magic!=MagicNum)
        {
            cout<<"魔数未初始化"<<endl;
            return;
        }
        this->nofree_qua.sem_V();

        mu.lock();
        //取出string
        string tem=this->ch_arry[write_to_file_index];//取出其中的信息
        this->ch_arry[write_to_file_index].clear();   //清除sting中的信息

        this->write_to_file_index=(write_to_file_index+1)%4;
        mu.unlock();
        this->free_qua.sem_P();
        {
            lock_guard<mutex> lock(file_mu);
            int ret=write(fd,tem.c_str(),tem.size());
            if(ret<0)
            {
                throw bad_alloc();
            }
            else
            {
                record_write_file++;
            }
        }
    }
}

void Logperf::set_cannot_used()
{
    magic=0;
}

void Logperf::set_can_used()
{
    magic=MagicNum;
}

void Logperf::change_file()
{
    char *cur_time = (char *)malloc(21*sizeof(char));
    gettime(cur_time);
    index++;
    string path_=file_name+cur_time+" "+to_string(index); //生成了new_path
    free(cur_time);
    cur_time=NULL;
    //初始化路径信息
    this->path=path_;
    {
        lock_guard<mutex> lock(file_mu);
        cout<<"change_file"<<endl;

        close(fd);
        fd=open(path_.c_str(),O_RDWR|O_CREAT,0777);
        if(fd==-1)
        {
            cout<<"open err"<<__LINE__<<__FUNCTION__<<endl;
            throw bad_alloc();
        }
    }
    return;
}

void Logperf::close_log()
{
     //当log被设置为不可用的时候 将剩余的内容写入到fd中
    lock_guard<mutex> lock(mu);
    //加锁之后 写入
    for(int i=0;i<4;i++)
    {
        if(ch_arry[i].size()!=0)
        {
            int ret=write(fd,ch_arry[i].c_str(),ch_arry[i].size());
            if(ret==-1)
            {
                cout<<"write err"<<__LINE__<<__FUNCTION__<<endl;
            }
            this->record_write_file++;
            ch_arry[i].clear();
        }
    }
}

int  Logperf::get_file_size()
{
    struct stat st;
    fstat (fd,&st);
    int ret=st.st_size/(1024*1024);
    return ret;
}

Logperf::~Logperf()
{
    set_cannot_used();
    close_log();
    //将剩余的内容写入到文件里面
    record_write_file=0;
    default_path.clear();
    path.clear();
    close(fd);
    write_to_file_index=0;
    free_index=0;
}

