//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CACHE_H
#define CACHE_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Elvis
{
  template <class C, class K, class V>
  class ICache
  {
  public:
    ICache() = default;

    void Insert(K key, V value)
    {
      return static_cast<C *>(this)->Insert(key, value);
    }

    V operator[](K const key) { return static_cast<C *>(this)->operator[](key); }

    void State() { return static_cast<C *>(this)->State(); }
  };

  template <class K, class V>
  class TimeCache : public ICache<TimeCache<K, V>, K, V>
  {
  public:
    TimeCache(int capacity) : m_Capacity(capacity){};

    void Insert(K key, V value)
    {
      std::lock_guard<std::mutex> guard(m_CacheLock);
      std::chrono::steady_clock::time_point Insertion_time =
          std::chrono::steady_clock::now();
      if (m_CacheIndex.find(key) == m_CacheIndex.end())
      {
        m_Cache.push_back(
            std::make_pair(Insertion_time, std::make_pair(key, value)));
        m_CacheIndex.Insert(std::make_pair(key, m_Cache.size() - 1));
      }
      else
      {
        m_Cache[m_CacheIndex[key]].first = Insertion_time;
        m_Cache[m_CacheIndex[key]].second.second = value;
      }

      if (m_Cache.size() > m_Capacity)
      {
        // O(nlog(n))
        std::sort(m_Cache.begin(), m_Cache.end(), std::greater<>());
        // O(n)
        for (int i = 0; i < m_Cache.size(); i++)
        {
          m_CacheIndex[m_Cache[i].second.first] = i;
        }
        K key_to_be_removed = m_Cache[m_Cache.size() - 1].second.first;
        m_CacheIndex.erase(key_to_be_removed);
        m_Cache.pop_back();
      }
      return;
    }

    V operator[](K const key)
    {
      std::lock_guard<std::mutex> guard(m_CacheLock);
      V output;
      if (m_CacheIndex.find(key) == m_CacheIndex.end())
      {
        return output;
      }
      else
      {
        std::chrono::steady_clock::time_point update_time =
            std::chrono::steady_clock::now();
        m_Cache[m_CacheIndex[key]].first = update_time;
        return m_Cache[m_CacheIndex[key]].second.second;
      }
    }

    void State()
    {
      auto end = std::chrono::steady_clock::now();
      for (auto it = m_Cache.begin(); it != m_Cache.end(); ++it)
      {
        std::cout << "entry Inserted before: "
                  << std::chrono::duration_cast<std::chrono::seconds>(end -
                                                                      it->first)
                         .count()
                  << " seconds key: " << it->second.first << "\n";
      }
    }

  private:
    int m_Capacity;
    std::mutex m_CacheLock;
    std::vector<std::pair<std::chrono::steady_clock::time_point, std::pair<K, V>>>
        m_Cache;
    std::unordered_map<K, int> m_CacheIndex;

    bool empty() const { return m_Cache.empty(); }

    int size() const { return m_Cache.size(); }
  };

  template <class K, class V>
  class LRUCache : public ICache<TimeCache<K, V>, K, V>
  {
  public:
    LRUCache(int capacity) : m_Capacity(capacity){};

    void Insert(K key, V value)
    {
      std::lock_guard<std::mutex> guard(m_CacheLock);
      if (m_CacheIndexes.find(key) != m_CacheIndexes.end())
      {
        auto m_CacheIndex = m_CacheIndexes[key];
        m_Cache.erase(m_CacheIndex);
      }
      else if (size() == m_Capacity)
      {
        auto key_to_remove = m_Cache.back().first;
        m_Cache.pop_back();
        m_CacheIndexes.erase(key_to_remove);
      }

      m_Cache.push_front(std::make_pair(key, value));
      auto m_CacheIndex = m_Cache.begin();
      m_CacheIndexes[key] = m_CacheIndex;
      return;
    }

    V operator[](K const key)
    {
      std::lock_guard<std::mutex> guard(m_CacheLock);
      V output;
      if (m_CacheIndexes.find(key) == m_CacheIndexes.end())
      {
        return output;
      }
      else
      {
        auto value = m_CacheIndexes[key]->second;
        auto m_CacheIndex = m_CacheIndexes[key];
        m_Cache.erase(m_CacheIndex);
        m_Cache.push_front(std::make_pair(key, value));
        m_CacheIndex = m_Cache.begin();
        m_CacheIndexes[key] = m_CacheIndex;
        return value;
      }
    }

    // To be used only for debug purposes.
    void State()
    {
      std::lock_guard<std::mutex> guard(m_CacheLock);
      std::cout << "Cache State\n";
      for (auto elem : m_Cache)
      {
        std::cout << elem.first << ":\n"
                  << elem.second << " |\n";
      }
      std::cout << "\n";
    }

  private:
    int m_Capacity;
    std::mutex m_CacheLock;
    std::list<std::pair<K, V>> m_Cache;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator>
        m_CacheIndexes;

    bool empty() const { return m_Cache.empty(); }

    int size() const { return m_Cache.size(); }
  };

  class ICacheManager
  {
  public:
    virtual ~ICacheManager() = default;

    template <class ICache>
    ICache *GetCache();
  };

  template <class CACHE>
  class CacheManager : public ICacheManager
  {
  public:
    CacheManager(int m_Cachesize)
    {
      m_Cache = std::make_unique<CACHE>(m_Cachesize);
    }

    CACHE *GetCache() { return m_Cache.get(); }

  private:
    std::unique_ptr<CACHE> m_Cache;
  };

  template <class ICache>
  ICache *ICacheManager::GetCache()
  {
    return static_cast<CacheManager<ICache> *>(this)->GetCache();
  }
} // namespace Elvis
#endif // CACHE_H
