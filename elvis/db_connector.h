#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <pqxx/pqxx>
#include <mutex>

template<class D, class R>
class db_connector
{
public:
    db_connector() = default;

    int select(std::string sql, R &mR) const
    {
        return static_cast<D *>(this)->select(sql, mR);
    }

    int insert(std::string sql) const
    {
        return static_cast<D *>(this)->insert(sql);
    }

    int update(std::string sql) const
    {
        return static_cast<D *>(this)->update(sql);
    }
};

class pg_connector : public db_connector<pg_connector, pqxx::result>
{
public:
    pg_connector(std::string dbname,
                 std::string user,
                 std::string password,
                 std::string hostaddr,
                 std::string port) : dbname_(dbname),
                                     user_(user),
                                     password_(password),
                                     hostaddr_(hostaddr),
                                     port_(port)
    {
        conn_info_ = "dbname = " + dbname_ + " user = " + user_ + " password = " + password_ +
                     " hostaddr = " + hostaddr_ + " port = " + port_;

        pg_connection_ = std::make_shared<pqxx::connection>(conn_info_);
        if (pg_connection_->is_open())
        {
            std::cout << "Opened connection to database successfully: " << pg_connection_->dbname() << "\n";
        }
        else
        {
            std::cout << "Can't open connection to database\n";
        }
    }

    int select(std::string sql, pqxx::result &mR) const
    {
        try
        {
            /* Create a non transactional object. */
            pqxx::nontransaction N(*pg_connection_);

            /* Execute SQL query */
            pqxx::result R(N.exec(sql));
            mR = R;
            //std::cout << "Operation done successfully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    int insert(std::string sql) const
    {
        try
        {
            /* Create a transactional object. */
            pqxx::work W(*pg_connection_);

            /* Execute SQL query */
            W.exec(sql);
            W.commit();
            //std::cout << "Operation done successfully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    int update(std::string sql) const
    {
        try
        {
            /* Create a transactional object. */
            pqxx::work W(*pg_connection_);

            /* Execute SQL query */
            W.exec(sql);
            W.commit();
            //std::cout << "Operation done successfully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }

private:
    std::string conn_info_;
    std::string dbname_, user_, password_, hostaddr_, port_;
    std::shared_ptr<pqxx::connection> pg_connection_;
};

class i_db_manager
{
public:
    virtual ~i_db_manager(){};

    template<class I_DB_C>
    std::unique_ptr<I_DB_C> access_db_connector();

    template<class I_DB_C>
    void return_db_connector(std::unique_ptr<I_DB_C> db_c);
};

template<class DB_C>
class db_manager: public i_db_manager
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

template<class I_DB_C>
std::unique_ptr<I_DB_C> i_db_manager::access_db_connector()
{
    return static_cast<db_manager<I_DB_C> *>(this)->access_db_connector();
}

template<class I_DB_C>
void i_db_manager::return_db_connector(std::unique_ptr<I_DB_C> db_c)
{
    static_cast<db_manager<I_DB_C> *>(this)->return_db_connector(std::move(db_c));
}

#endif //DB_CONNECTOR_H