//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef RESPONSE_CONTEXT_H
#define RESPONSE_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>

class response_creator{
public:
  response_creator() = default;

  virtual std::string create_response(std::string && input_data, std::string && status) = 0;

};

class http_response_creator: public response_creator{
public:
  http_response_creator() = default;

  std::string create_response(std::string && input_data, std::string && status) override;
};

class i_response_context{
public:
  i_response_context() = default;
  
  void set_response_context(std::unique_ptr<response_creator> response){
    response_ = std::move(response);
  }
  
  std::string do_create_response(std::string && input_data, std::string && status){
    return response_->create_response(std::move(input_data), std::move(status));
  }

protected:
  std::unique_ptr<response_creator> response_;
};

class http_response_context: public i_response_context{
public:
  http_response_context(){
    this->response_ = std::make_unique<http_response_creator>(); //default for now
  }
};

#endif //RESPONSE_CONTEXT_H
