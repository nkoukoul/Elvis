//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include "controllers.h"

class route_manager{
public:
  route_manager() = default;

  i_controller * get_controller(std::string url, std::string request_type){
    if (get_route(url, request_type)){
      //std::cout << "url " << url << " request " << request_type << " exists\n";
      return route_map[url][request_type].get();
    }else{
      //std::cout << "url " << url << " request " << request_type << " does not exist will try with suburl\n";
      std::size_t index = url.find_last_of("/");
      url = url.substr(0, index > 0 ? index : 0);
      if (get_route(url, request_type)){
	std::cout << "partial_url " << url << " request " << request_type << " exists\n";
	return route_map[url][request_type].get();
      }
    }
    //std::cout << "url " << url << " request " << request_type << " does not exist\n";
    return {};
  }
  
  void set_route(std::string url, std::string request_type, std::unique_ptr<i_controller> controller){
    if (route_map.find(url) == route_map.end()){
      //std::pair<std::string, std::unique_ptr<i_controller>> rt_pair = ;
      std::unordered_map<std::string, std::unique_ptr<i_controller>> request_types;
      request_types.insert(std::move(std::make_pair(request_type, std::move(controller))));
      route_map.insert(std::move(std::make_pair(url, std::move(request_types)))); 
    } else if (route_map[url].find(request_type) == route_map[url].end()){
      route_map[url].insert(std::move(std::make_pair(request_type, std::move(controller))));
    } else {
      std::cout << url<< " has already a request type " << request_type << " registered\n";
    }
    return;
  }

  bool get_route(std::string url, std::string request_type){
    return route_map.find(url) != route_map.end() && route_map[url].find(request_type) != route_map[url].end();
  }

private:
  std::mutex route_manager_lock_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<i_controller>>> route_map;
};


#endif //ROUTE_MANAGER_H
