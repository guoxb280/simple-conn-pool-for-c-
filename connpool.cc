#include "connpool.h"

ConnPool* ConnPool::connPool=NULL; 
pthread_mutex_t ConnPool::lock_get_instance = PTHREAD_MUTEX_INITIALIZER;

//连接池的构造函数  
ConnPool::ConnPool(string url, string username,string password, string database, int maxSize)  
{
    pthread_mutex_init(&lock,NULL);  
    this->maxSize=maxSize; 
    this->curSize=0;  
    this->username= const_cast<char*>(username.c_str());  
    this->password=const_cast<char*>(password.c_str());  
    this->url=const_cast<char*>(url.c_str()); 
    this->database=const_cast<char*>(database.c_str()); 
    this->InitConnection(maxSize/2);  
}  
// in data_head.h
// #define CONFIG_instanceId "instanceId"
// #define CONFIG_serverIp "serverIp"
// #define CONFIG_serverPort "serverPort"
// #define CONFIG_DB "DB"
// #define CONFIG_DBIp "DBIp"
// #define CONFIG_DBUser "DBUser"
// #define CONFIG_DBPassword "DBPassword"
// #define CONFIG_DBConnInitialSize "DBConnInitialSize"
// #define CONFIG_tagCacheTableInitialSize "tagCacheTableInitialSize"
//获取连接池对象，单例模式  
ConnPool*ConnPool::GetInstance(){  
    pthread_mutex_lock(&lock_get_instance);
    if(connPool==NULL)  
    {  
        connPool=new ConnPool(SYSTEM_CONFIG_DBIp,SYSTEM_CONFIG_DBUser
            ,SYSTEM_CONFIG_DBPassword, SYSTEM_CONFIG_DB, SYSTEM_CONFIG_DBConnInitialSize);  
    }  
    pthread_mutex_unlock(&lock_get_instance);  
    return connPool;  
}  
//初始化连接池，创建最大连接数的一半连接数量  
void ConnPool::InitConnection(int iInitialSize)  
{  
    MYSQL* conn;  
    int rt_code = pthread_mutex_lock(&lock);
    for(int i=0;i<iInitialSize;i++)  
    {  
        conn=this->CreateConnection();  
        if(conn){  
            connList.push_back(conn);  
            ++(this->curSize);  
        }  
        else  
        {  
            fprintf(stderr, "%s\n", mysql_error(conn));
        }  
    }  
    pthread_mutex_unlock(&lock);  
}  
//创建连接,返回一个Connection  
MYSQL*ConnPool::CreateConnection()
{  
    MYSQL* conn;  
    conn = mysql_init(NULL);
   if (!mysql_real_connect(conn, this->url,
          this->username, this->password, this->database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
   }
   return conn;
}  

//在连接池中获得一个连接  
MYSQL*ConnPool::GetConnection(){ 
    //printf("get_connection ===============\n");
    //return NULL;
    // int rt_code = pthread_mutex_lock(&lock);
    // MYSQL* conn;  
    // conn = mysql_init(NULL);
    
    // if (!mysql_real_connect(conn, this->url,
    //         this->username, this->password, this->database, 0, NULL, 0)) {
    //     fprintf(stderr, "%s\n", mysql_error(conn));
    // }
    // pthread_mutex_unlock(&lock);  
    // return conn;
    MYSQL* conn;  
    pthread_mutex_lock(&lock);
    //printf("size_getConn %d\n", connList.size());
    if(connList.size()>0)//连接池容器中还有连接  
    {  
        conn=connList.front();//得到第一个连接  
        connList.pop_front();//移除第一个连接  
        /*if(conn->isClosed)//如果连接已经被关闭，删除后重新建立一个  
        {  
            delete conn;  
            conn=this->CreateConnection();  
        }  */
        //如果连接为空，则创建连接出错  
        if(conn==NULL)  
        {  
            --curSize;  
        }  
        pthread_mutex_unlock(&lock);  
        return conn;  
    }  
    else{  
        if(curSize< maxSize){//还可以创建新的连接  
            conn= this->CreateConnection();  
            if(conn){  
                ++curSize;  
                pthread_mutex_unlock(&lock);  
                return conn;  
            }  
            else{  
                pthread_mutex_unlock(&lock);  
                return NULL;  
            }  
        }  
        else{//建立的连接数已经达到maxSize  
            pthread_mutex_unlock(&lock);  
            return NULL;  
        }  
    }  
}  
//回收数据库连接  
void ConnPool::ReleaseConnection(MYSQL* conn){  

    // if(conn){  
    //     this->DestoryConnection(conn);  
    // }
    // return ;//debug

    if(conn){  
        pthread_mutex_lock(&lock);  
        //printf("size_releaseConn %d\n", connList.size());
        connList.push_back(conn);  
        pthread_mutex_unlock(&lock);  
    }  
}  
//连接池的析构函数  
ConnPool::~ConnPool()  
{  
    this->DestoryConnPool();  
}  
//销毁连接池,首先要先销毁连接池的中连接  
void ConnPool::DestoryConnPool(){  
    list<MYSQL*>::iterator icon;  
    pthread_mutex_lock(&lock);  
    for(icon=connList.begin();icon!=connList.end();++icon)  
    {  
        this->DestoryConnection(*icon);//销毁连接池中的连接  
    }  
    curSize=0;  
    connList.clear();//清空连接池中的连接  
    pthread_mutex_unlock(&lock);  
}  
//销毁一个连接  
void ConnPool::DestoryConnection(MYSQL* conn)  
{  
    if(conn)  
    {  
        try{  
            mysql_close(conn);  
        }  
        catch(exception & e)  
        {  
            fprintf(stderr, "%s %s\n", e.what(), mysql_error(conn));
        }  
        delete conn;  
    }  
}  

/*
string db_get_id(MYSQL *conn, char x_query[])
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    string id = "";
    myquery_rt_str(conn, x_query);
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL)
        id = row[0];
    mysql_free_result(res);
    return id;
}

int db_count(string table_name)
{
        //printf("-------------\n");
    ConnPool* cp = ConnPool::GetInstance();
        //printf("+++++\n");
    MYSQL *conn = cp->GetConnection();
        //printf("=====\n");
    if (conn == NULL) return 0;
    MYSQL_ROW row;
    MYSQL_RES *res;
    char x_query[QUERY_STR_MAX_LEN];
    int cnt = 0;
    str_count(x_query, table_name);
    if (mysql_query(conn, x_query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        cp->ReleaseConnection(conn);
        return 0;
    }
    res = mysql_use_result(conn);
    if ((row=mysql_fetch_row(res)) != NULL)
        cnt = str_to_int(row[0]);
    mysql_free_result(res);
    cp->ReleaseConnection(conn);
    return cnt;
}
*/

