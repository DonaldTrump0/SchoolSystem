#ifdef MYSQLDLL_EXPORTS
#define MYSQLDLL_API __declspec(dllexport)
#else
#define MYSQLDLL_API __declspec(dllimport)
#endif

#pragma once
#include <string>
#include "../MySQL Server 5.7/include/mysql.h"
#pragma comment(lib, "../MySQL Server 5.7/lib/libmysql.lib")
using namespace std;

class MYSQLDLL_API CMysql
{
public:
    CMysql();
    ~CMysql();

    bool Init(const char* host, const char* user, const char* password);

    // ִ��sql��䣬����Ӱ�������
    int Query(string sql);
    int Query(string sql, MYSQL_RES*& res);

private:
    MYSQL* m_mysql;
};