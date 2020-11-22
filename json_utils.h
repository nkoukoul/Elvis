#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <unordered_map>
#include <string>
#include <stack>
#include <list>
#include "models.h"

class deserialize_strategy
{
public:
  deserialize_strategy() = default;
  virtual std::list<std::unordered_map<std::string, std::string>> deserialize(std::string && input) const = 0;
};


class nkou_deserialize_strategy : public deserialize_strategy
{
public:
  std::list<std::unordered_map<std::string, std::string>> deserialize(std::string && input) const override;
};


class json_util_context{
public:
  json_util_context(){
    this->ds_ = new nkou_deserialize_strategy();//default for now
  }
  
  void do_deserialize(std::string && input, model * c_model){
    auto desirialized_input = ds_->deserialize(std::move(input));
    c_model->model_map(std::move(desirialized_input));
  }
  
  void set_deserialize_strategy(deserialize_strategy * ds){
    delete this->ds_;
    this->ds_ = ds;
  }

private:
  deserialize_strategy * ds_;
};



#endif //JSON_UTILS_H
