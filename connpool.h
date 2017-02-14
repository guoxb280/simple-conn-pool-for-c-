#ifndef _CONNPOOL_H
#define _CONNPOOL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <list>

#include <pthread.h>//for __thread

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <inttypes.h>  

#include <mysql/mysql.h>
#include <pthread.h>
#include <stdexcept>  
#include <exception>  
#include <iostream>

using namespace std;

//=======SYSTEM CONFIG==========Begin
//#define SYSTEM_CONFIG_instanceId "11111111"
//#define SYSTEM_CONFIG_serverIp "0.0.0.0"
//#define SYSTEM_CONFIG_serverPort "12345"

#define SYSTEM_CONFIG_DB "label_server_db"
#define SYSTEM_CONFIG_DBIp "localhost"
#define SYSTEM_CONFIG_DBUser "root"
#define SYSTEM_CONFIG_DBPassword "difc123"
#define SYSTEM_CONFIG_DBConnInitialSize 50

//#define SYSTEM_CONFIG_tagCacheTableInitialSize 50
//#define QUERY_STR_MAX_LEN 10000
//=======SYSTEM CONFIG==========End

inline unsigned long long str_to_ull(string s)
{
    unsigned long long rt = 0ULL;
    for (int i = 0; i<(int)s.size(); i++)
        rt = rt * 10 + (s[i]-'0')*1;
    return rt;
}
inline int str_to_int(string s)
{
	int rt = 0;
    for (int i = 0; i<(int)s.size(); i++)
        rt = rt * 10 + (s[i]-'0')*1;
    return rt;
}

class ConnPool{
  private:
    int curSize;//当前已建立的数据库连接数量
    int maxSize;//连接池中定义的最大数据库连接数
    char *username;
    char *password;
    char *url;
    char *database;
    list<MYSQL*> connList;//连接池的容器队列
    pthread_mutex_t lock;//线程锁
    static ConnPool *connPool;
    //Driver* driver;

    MYSQL* CreateConnection();//创建一个连接
    void DestoryConnection(MYSQL *conn);//销毁数据库连接对象
    void InitConnection(int iInitialSize);//初始化数据库连接池
    void DestoryConnPool();//销毁数据库连接池
    ConnPool(string url, string username,string password, string database, int maxSize);//构造方法
  public:
    ~ConnPool();
    MYSQL* GetConnection();//获得数据库连接
    void ReleaseConnection(MYSQL *conn);//将数据库连接放回到连接池的容器中
    static ConnPool *GetInstance();//获取数据库连接池对象
    static pthread_mutex_t lock_get_instance;//线程锁
};

#define myquery_release_rt(cp, conn, x_query) \
        if (mysql_query(conn, x_query)) \
        { \
            fprintf(stderr, "%s\n", mysql_error(conn)); \
            cp->ReleaseConnection(conn); \
			return -1; \
        } 

#define myquery_rt(conn, x_query) \
        if (mysql_query(conn, x_query)) \
        { \
            fprintf(stderr, "%s\n", mysql_error(conn)); \
            return -1; \
        } 
#define myquery_rt_str(conn, x_query) \
        if (mysql_query(conn, x_query)) \
        { \
            fprintf(stderr, "%s\n", mysql_error(conn)); \
            return ""; \
        } 
#define myquery_nort(conn, x_query) \
        if (mysql_query(conn, x_query)) \
        { \
            fprintf(stderr, "%s\n", mysql_error(conn)); \
        } 

#define str_count(x_query, table_name) \
    sprintf(x_query, "select count(*) from %s", (table_name).c_str())

//inline string db_get_id(MYSQL *conn, char x_query[]);
//int db_count(string table_name);

#endif
