#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <pqxx/pqxx>
#include <mutex>

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

template<class DB_C>
class db_manager
{
public:
    db_manager(int connection_num)
    {
        for (auto i = connection_num; i > 0; --i)
        {
            db_connection_pool_.emplace_back(std::make_unique<DB_C>("test_db", "test", "test", "127.0.0.1", "5432"));
        }
    }

    std::unique_ptr<DB_C> access_db_connector()
    {
        std::lock_guard<std::mutex> _lock(db_lock_);
        if (db_connection_pool_.empty())
        {
            return nullptr;
        }
        auto db_c = std::move(db_connection_pool_.back());
        db_connection_pool_.pop_back();
        return std::move(db_c);
    }

    void return_db_connector(std::unique_ptr<DB_C> db_c)
    {
        std::lock_guard<std::mutex> _lock(db_lock_);
        db_connection_pool_.push_back(std::move(db_c));
    }

private:
    std::mutex db_lock_;
    std::vector<std::unique_ptr<DB_C>> db_connection_pool_;
};

#endif //DB_CONNECTOR_H