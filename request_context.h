//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>

class request_parser{
public:
  request_parser() = default;

  virtual std::unordered_map<std::string, std::string> parse(std::string && input_data) = 0;

};

class http_request_parser: public request_parser{
public:
  http_request_parser() = default;

  std::unordered_map<std::string, std::string> parse(std::string && input_data) override;
};

class i_request_context{
public:
  i_request_context() = default;
  
  void set_request_context(std::unique_ptr<request_parser> request){
    request_ = std::move(request);
  }
  
  std::unordered_map<std::string, std::string>  do_parse(std::string && input_data){
    return request_->parse(std::move(input_data));
  }

protected:
  std::unique_ptr<request_parser> request_;
};

class http_request_context: public i_request_context{
public:
  http_request_context(){
    this->request_ = std::make_unique<http_request_parser>(); //default for now
  }
};

#endif //REQUEST_CONTEXT_H
