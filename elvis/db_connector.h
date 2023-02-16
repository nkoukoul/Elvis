//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <mutex>

namespace Elvis
{
    template <class D, class R>
    class DBConnector
    {
    public:
        virtual ~DBConnector() = default;

        int Select(std::string sql, R &mR) const
        {
            return static_cast<D *>(this)->Select(sql, mR);
        }

        int Insert(std::string sql) const
        {
            return static_cast<D *>(this)->Insert(sql);
        }

        int Update(std::string sql) const
        {
            return static_cast<D *>(this)->Update(sql);
        }
    };

    class MockConnector : public DBConnector<MockConnector, std::string>
    {
    public:
        MockConnector() = default;

        MockConnector(std::string dbname,
                      std::string user,
                      std::string password,
                      std::string hostaddr,
                      std::string port) {}

        int Select(std::string sql, std::string &mR) const
        {
            return 0;
        }

        int Insert(std::string sql) const
        {
            return 0;
        }

        int Update(std::string sql) const
        {
            return 0;
        }
    };

    class IDBManager
    {
    public:
        virtual ~IDBManager(){};

        template <class I_DB_C>
        std::unique_ptr<I_DB_C> ExtractDBConnectorFromPool();

        template <class I_DB_C>
        void ReturnDBConnectorToPool(std::unique_ptr<I_DB_C> db_c);
    };

    template <class DB_C>
    class DBManager : public IDBManager
    {
    public:
        DBManager(int connection_num)
        {
            for (auto i = connection_num; i > 0; --i)
            {
                m_DBConnectionPool.emplace_back(std::make_unique<DB_C>("test_db", "test", "test", "127.0.0.1", "5432"));
            }
        }

        std::unique_ptr<DB_C> ExtractDBConnectorFromPool()
        {
            std::lock_guard<std::mutex> lock(m_DBLock);
            if (m_DBConnectionPool.empty())
            {
                return nullptr;
            }
            auto db_c = std::move(m_DBConnectionPool.back());
            m_DBConnectionPool.pop_back();
            return std::move(db_c);
        }

        void ReturnDBConnectorToPool(std::unique_ptr<DB_C> db_c)
        {
            std::lock_guard<std::mutex> lock(m_DBLock);
            m_DBConnectionPool.push_back(std::move(db_c));
        }

    private:
        std::mutex m_DBLock;
        std::vector<std::unique_ptr<DB_C>> m_DBConnectionPool;
    };

    template <class I_DB_C>
    std::unique_ptr<I_DB_C> IDBManager::ExtractDBConnectorFromPool()
    {
        return static_cast<DBManager<I_DB_C> *>(this)->ExtractDBConnectorFromPool();
    }

    template <class I_DB_C>
    void IDBManager::ReturnDBConnectorToPool(std::unique_ptr<I_DB_C> db_c)
    {
        static_cast<DBManager<I_DB_C> *>(this)->ReturnDBConnectorToPool(std::move(db_c));
    }

    class IDBEngine
    {
    public:
        virtual ~IDBEngine() = default;

        virtual void CreateModel(std::string query) = 0;

        virtual void RetrieveModel(std::string query) = 0;
    };

    class MockEngine : public IDBEngine
    {
    private:
        std::unique_ptr<IDBManager> m_DBManager;

    public:
        MockEngine(int threads)
        {
            m_DBManager = std::make_unique<DBManager<MockConnector>>(threads);
        }

        virtual void CreateModel(std::string query) override
        {
            auto db_c = m_DBManager->ExtractDBConnectorFromPool<MockConnector>();
            db_c->Insert(query);
            m_DBManager->ReturnDBConnectorToPool<MockConnector>(std::move(db_c));
        }

        virtual void RetrieveModel(std::string query) override
        {
            std::string query_result;
            auto db_c = m_DBManager->ExtractDBConnectorFromPool<MockConnector>();
            db_c->Select(query, query_result);
            m_DBManager->ReturnDBConnectorToPool<MockConnector>(std::move(db_c));
        }
    };
}
#endif // DB_CONNECTOR_H