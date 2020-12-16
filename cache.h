//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//


#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <list>
#include <unordered_map>
#include <mutex>

template<class K, class V>
class cache{
public:
  cache(int capacity):capacity_(capacity){};
  
  void insert(std::pair<K,V> && k_v_pair){
    std::lock_guard<std::mutex> guard(cache_lock_);
    if (cache_map_.find(k_v_pair.first) == cache_map_.end()){
      K prev_begin_key;
      
      if (!cache_list_.empty()){
	prev_begin_key = cache_list_.front().first;
      }
      
      cache_list_.push_front(k_v_pair);
      
      if (cache_list_.size() > 1){
	auto prev_begin_key_it = std::next(cache_list_.begin());
	cache_map_[prev_begin_key] = prev_begin_key_it;
      }      
      cache_map_.insert(std::make_pair(k_v_pair.first, cache_list_.begin()));
      if (cache_list_.size() > capacity_){
	K key_to_be_removed = cache_list_.back().first;
	cache_map_.erase(key_to_be_removed);
	cache_list_.pop_back();
      }
    }
    return;
  }

  bool find(K const key){
    std::lock_guard<std::mutex> guard(cache_lock_);
    return cache_map_.find(key) != cache_map_.end();
  }
  
  std::pair<K,V> & operator [](K key){
    std::lock_guard<std::mutex> guard(cache_lock_);
    K prev_begin_key = cache_list_.front().first;
    auto it = cache_map_[key];
    std::pair<K,V> new_first_pair = *it;
    cache_list_.erase(it);
    cache_list_.push_front(new_first_pair);
    auto prev_begin_key_it = std::next(cache_list_.begin());
    cache_map_[prev_begin_key] = prev_begin_key_it;
    cache_map_[key] = cache_list_.begin();
    //std::swap(cache_map_[key]->second, cache_map_[prev_begin_key]->second);
    return cache_list_.front();
  }

  void state(){
    std::lock_guard<std::mutex> guard(cache_lock_);
    for (auto it = cache_list_.begin(); it != cache_list_.end(); ++it){
      std::cout << "key: " << it->first << " value: " << it->second << "\n";
    }
  }

private:
  int capacity_;
  std::mutex cache_lock_;
  std::list<std::pair<K, V>> cache_list_;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cache_map_;

  bool empty() const {
    return cache_list_.empty();
  }

  int size() const {
    return cache_list_.size();
  }
};

#endif //CACHE_H
