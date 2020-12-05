#include "CMysql.h"
#include <stdio.h>

void OutputDebugPrintf(const char* strOutputString, ...)
{
    char strBuffer[4096] = { 0 };
    va_list vlArgs;
    va_start(vlArgs, strOutputString);
    vsprintf(strBuffer, strOutputString, vlArgs);
    va_end(vlArgs);
    OutputDebugString(strBuffer);
}

CMysql::CMysql() : m_mysql(NULL)
{
}

CMysql::~CMysql()
{
    mysql_close(m_mysql);
    mysql_library_end();
}

bool CMysql::Init(const char* host, const char* user, const char* password)
{
    // 初始化库
    if (0 != mysql_library_init(0, NULL, NULL))
    {
        OutputDebugPrintf("could not initialize MySQL client library\n");
        return false;
    }

    // 初始化MYSQL结构体
    m_mysql = mysql_init(NULL);
    if (NULL == m_mysql)
    {
        OutputDebugPrintf("mysql_init: %s\n", mysql_error(m_mysql));
        return false;
    }

    // 设置字符集
    if (0 != mysql_set_character_set(m_mysql, "gbk"))
    {
        OutputDebugPrintf("mysql_set_character_set: %s\n", mysql_error(m_mysql));
        return false;
    }

    // 连接
    if (NULL == mysql_real_connect(m_mysql, host, user, password, NULL, 3306, NULL, 0))
    {
        OutputDebugPrintf("mysql_real_connect: %s\n", mysql_error(m_mysql));
        return false;
    }

    return true;
}

int CMysql::Query(string sql)
{
    if (0 != mysql_query(m_mysql, sql.c_str()))
    {
        OutputDebugPrintf("mysql_query: %s\n", mysql_error(m_mysql));
        return -1;
    }

    return mysql_affected_rows(m_mysql);
}

int CMysql::Query(string sql, MYSQL_RES*& res)
{
    if (0 != mysql_query(m_mysql, sql.c_str()))
    {
        OutputDebugPrintf("mysql_query: %s\n", mysql_error(m_mysql));
        return -1;
    }

    //res = mysql_store_result(m_mysql);
    res = mysql_use_result(m_mysql);

    return mysql_affected_rows(m_mysql);
}