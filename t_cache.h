//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//


#ifndef T_CACHE_H
#define T_CACHE_H

#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <mutex>

template<class K, class V>
class t_cache{
public:
  t_cache(int capacity):capacity_(capacity){};
    
  void insert(std::pair<K,V> && k_v_pair){
    std::lock_guard<std::mutex> guard(cache_lock_);
    std::chrono::steady_clock::time_point insertion_time = std::chrono::steady_clock::now();
    if (cache_index_.find(k_v_pair.first) == cache_index_.end()){
      cache_.push_back(std::make_pair(insertion_time, k_v_pair));
      cache_index_.insert(std::make_pair(k_v_pair.first, cache_.size()-1));
    } else {
      cache_[cache_index_[k_v_pair.first]].first = insertion_time;
    }
    
    if (cache_.size() > capacity_){
      //O(nlog(n))
      std::sort(cache_.begin(), cache_.end(), std::greater <>());
      //O(n)
      for (int i = 0; i < cache_.size(); i++){
	cache_index_[cache_[i].second.first] = i;
      }
      K key_to_be_removed = cache_[cache_.size()-1].second.first;
      cache_index_.erase(key_to_be_removed);
      cache_.pop_back();
    }
    return;
  }

  bool find(K const key){
    std::lock_guard<std::mutex> guard(cache_lock_);
    return cache_index_.find(key) != cache_index_.end();
  }
  
  std::pair<K,V> operator [](K const key) {
    std::lock_guard<std::mutex> guard(cache_lock_);
    if (cache_index_.find(key) == cache_index_.end()) return {}; 
    return cache_[cache_index_[key]].second;
  }

  void state(){
    std::lock_guard<std::mutex> guard(cache_lock_);
    auto end = std::chrono::steady_clock::now();
    for (auto it = cache_.begin(); it != cache_.end(); ++it){
      std::cout << "entry inserted before: " << std::chrono::duration_cast<std::chrono::seconds>(end - it->first).count() << " seconds key: " << it->second.first << " value: " << it->second.second << "\n";
    }
  }

private:
  int capacity_;
  std::mutex cache_lock_;
  std::vector<std::pair<std::chrono::steady_clock::time_point, std::pair<K, V>>> cache_;
  std::unordered_map<K, int> cache_index_;

  bool empty() const {
    return cache_.empty();
  }
  
  int size() const {
    return cache_.size();
  }

};

#endif //T_CACHE_H
