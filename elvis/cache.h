//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <mutex>
#include <algorithm>
#include <list>
#include <vector>
#include <chrono>
#include <unordered_map>

template<class C, class K, class V>
class i_cache
{
public:
  i_cache() = default;
  
  void insert(K key, V value)
  {
    return static_cast<C *>(this)->insert(key, value);
  }
  
  V operator[](K const key)
  {
    return static_cast<C *>(this)->operator[](key);
  }
  
  void state()
  {
    return static_cast<C *>(this)->state();
  }
};

template <class K, class V>
class t_cache : public i_cache<t_cache<K, V>, K, V>
{
public:
  t_cache(int capacity) : capacity_(capacity){};

  void insert(K key, V value)
  {
    std::lock_guard<std::mutex> guard(cache_lock_);
    std::chrono::steady_clock::time_point insertion_time = std::chrono::steady_clock::now();
    if (cache_index_.find(key) == cache_index_.end())
    {
      cache_.push_back(std::make_pair(insertion_time, std::make_pair(key, value)));
      cache_index_.insert(std::make_pair(key, cache_.size() - 1));
    }
    else
    {
      cache_[cache_index_[key]].first = insertion_time;
      cache_[cache_index_[key]].second.second = value; 
    }

    if (cache_.size() > capacity_)
    {
      //O(nlog(n))
      std::sort(cache_.begin(), cache_.end(), std::greater<>());
      //O(n)
      for (int i = 0; i < cache_.size(); i++)
      {
        cache_index_[cache_[i].second.first] = i;
      }
      K key_to_be_removed = cache_[cache_.size() - 1].second.first;
      cache_index_.erase(key_to_be_removed);
      cache_.pop_back();
    }
    return;
  }

  V operator[](K const key)
  {
    std::lock_guard<std::mutex> guard(cache_lock_);
    V output;
    if (cache_index_.find(key) == cache_index_.end())
    {
      return output;
    }
    else
    {
      std::chrono::steady_clock::time_point update_time = std::chrono::steady_clock::now();
      cache_[cache_index_[key]].first = update_time;
      return cache_[cache_index_[key]].second.second;
    }
  }

  void state()
  {
    auto end = std::chrono::steady_clock::now();
    for (auto it = cache_.begin(); it != cache_.end(); ++it)
    {
      std::cout << "entry inserted before: "
                << std::chrono::duration_cast<std::chrono::seconds>(end - it->first).count()
                << " seconds key: " << it->second.first << "\n";
    }
  }

private:
  int capacity_;
  std::mutex cache_lock_;
  std::vector<std::pair<std::chrono::steady_clock::time_point, std::pair<K, V>>> cache_;
  std::unordered_map<K, int> cache_index_;

  bool empty() const
  {
    return cache_.empty();
  }

  int size() const
  {
    return cache_.size();
  }
};

template <class K, class V>
class lru_cache : public i_cache<t_cache<K, V>, K, V>
{
public:
    lru_cache(int capacity) : capacity_(capacity) {};

    void insert(K key, V value)
    {
        std::lock_guard<std::mutex> guard(cache_lock_);
        if (cache_indexes_.find(key) != cache_indexes_.end())
        {
            auto cache_index = cache_indexes_[key];
            cache_.erase(cache_index);
        }
        else if (size() == capacity_)
        {
            auto key_to_remove = cache_.back().first;
            cache_.pop_back();
            cache_indexes_.erase(key_to_remove);
        }
        
        cache_.push_front(std::make_pair(key, value));
        auto cache_index = cache_.begin();
        cache_indexes_[key] = cache_index;
        return;
    }

    V operator[](K const key)
    {
        std::lock_guard<std::mutex> guard(cache_lock_);
        V output;
        if (cache_indexes_.find(key) == cache_indexes_.end())
        {
            return output;
        }
        else
        {
            auto value = cache_indexes_[key]->second;
            auto cache_index = cache_indexes_[key];
            cache_.erase(cache_index);
            cache_.push_front(std::make_pair(key, value));
            cache_index = cache_.begin();
            cache_indexes_[key] = cache_index;
            return value;
        }
    }

    void state()
    {
        std::cout << "cache state \n";
        for (auto elem : cache_)
        {
            std::cout << elem.first << ": " << elem.second << " | ";

        }
        std::cout << "\n";
    }

private:
    int capacity_;
    std::mutex cache_lock_;
    std::list<std::pair<K, V>> cache_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cache_indexes_;

    bool empty() const
    {
        return cache_.empty();
    }

    int size() const
    {
        return cache_.size();
    }
};


class i_cache_manager
{
public:
  virtual ~i_cache_manager(){};
  
  template<class I_CACHE>
  I_CACHE *access_cache();
};

template<class CACHE>
class cache_manager : public i_cache_manager
{
public:
  cache_manager(int cache_size)
  {
    cache_ = std::make_unique<CACHE>(cache_size);
  }

  CACHE *access_cache()
  {
    return cache_.get();
  }

private:
  std::unique_ptr<CACHE> cache_;
};

template <class I_CACHE>
I_CACHE *i_cache_manager::access_cache()
{
  return static_cast<cache_manager<I_CACHE> *>(this)->access_cache();
}

#endif //CACHE_H
