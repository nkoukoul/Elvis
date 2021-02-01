#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <pqxx/pqxx>

class db_connector
{
public:
    db_connector() = default;

    virtual int pg_select(std::string sql, pqxx::result &mR) const = 0;

    virtual int pg_insert(std::string sql) const = 0;

    virtual int pg_update(std::string sql) const = 0;
};

class pg_connector : public db_connector
{
public:
    pg_connector(std::string dbname,
                 std::string user,
                 std::string password,
                 std::string hostaddr,
                 std::string port);
    // arg[0] = table name
    // arg[1] = column name
    // arg[2] = order by
    int pg_select(std::string sql, pqxx::result &mR) const override;
    
    int pg_insert(std::string sql) const override;
    
    int pg_update(std::string sql) const override;
private:
    std::string conn_info_;
    std::string dbname_, user_, password_, hostaddr_, port_;
    std::shared_ptr<pqxx::connection> pg_connection_;
};

#endif //DB_CONNECTOR_H