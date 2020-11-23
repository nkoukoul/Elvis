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
#include <unordered_set>
#include <string>

class route_manager{
public:
  route_manager() = default;
  
  void set_route(std::string url, std::string request_type){
    if (route_map.find(url) == route_map.end()){
      std::unordered_set<std::string> request_types = {request_type};
      route_map.insert(std::make_pair(url, request_types)); 
    }else{
      std::unordered_set<std::string> & request_types = route_map[url];
      if (request_types.find(request_type) == request_types.end()){
	request_types.insert(request_type);
      }
      else{
	std::cout << url<< " has already a request type " << request_type << " registered\n";
      }
    }
    return;
  }

  bool get_route(std::string url, std::string request_type){
    return route_map.find(url) != route_map.end() && route_map[url].find(request_type) != route_map[url].end();
  }

private:
  std::unordered_map<std::string, std::unordered_set<std::string>> route_map;
};


#endif //ROUTE_MANAGER_H
