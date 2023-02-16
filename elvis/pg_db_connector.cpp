#ifndef PG_DB_CONNECTOR_H
#define PG_DB_CONNECTOR_H

#include "db_connector.h"
#include <pqxx/pqxx>

class pg_connector : public Elvis::DBConnector<pg_connector, pqxx::result>
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

    int Select(std::string sql, pqxx::result &mR) const
    {
        try
        {
            /* Create a non transactional object. */
            pqxx::nontransaction N(*pg_connection_);

            /* Execute SQL query */
            pqxx::result R(N.exec(sql));
            mR = R;
            // std::cout << "Operation done successfully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    int Insert(std::string sql) const
    {
        try
        {
            /* Create a transactional object. */
            pqxx::work W(*pg_connection_);

            /* Execute SQL query */
            W.exec(sql);
            W.commit();
            // std::cout << "Operation done successfully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    int Update(std::string sql) const
    {
        try
        {
            /* Create a transactional object. */
            pqxx::work W(*pg_connection_);

            /* Execute SQL query */
            W.exec(sql);
            W.commit();
            // std::cout << "Operation done successfully\n";
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

class PGEngine : public Elvis::IDBEngine
{
private:
    std::shared_ptr<DBManager<pg_connector>> m_DBManager;
public:
    PGEngine(int threads)
    {
        m_DBManager = std::make_unique<DBManager<pg_connector>>(threads);
    }

    virtual void CreateModel(std::string query) override
    {
        auto db_c = m_DBManager->ExtractDBConnectorFromPool<pg_connector>();
        db_c->Insert(query);
        m_DBManager->ReturnDBConnectorToPool<pg_connector>(std::move(db_c));
    }

    void RetrieveModel(std::string query) override
    {
        pqxx::result query_result;
        auto db_c = m_DBManager->ExtractDBConnectorFromPool<pg_connector>();
        db_c->Select(query, query_result);
        for (pqxx::result::const_iterator c = query_result.begin(); c != query_result.end(); ++c)
        {
            filename_.set(c[1].as<std::string>());
            md5sum_.set(c[2].as<std::string>());
        }
        m_DBManager->ReturnDBConnectorToPool<pg_connector>(std::move(db_c));
    }
};

#endif // PG_DB_CONNECTOR_H