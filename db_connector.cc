#include "db_connector.h"

pg_connector::pg_connector(std::string dbname,
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

int pg_connector::pg_select(std::string sql, pqxx::result &mR) const
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

int pg_connector::pg_insert(std::string sql) const
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

int pg_connector::pg_update(std::string sql) const
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