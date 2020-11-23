#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <unordered_map>
#include <string>
#include <stack>
#include <list>
#include <memory>
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

class i_json_util_context{
public:
  i_json_util_context() = default;
  
  std::list<std::unordered_map<std::string, std::string>> do_deserialize(std::string && input) const {
    return ds_->deserialize(std::move(input));
  }
  
  void set_deserialize_strategy(std::unique_ptr<deserialize_strategy> ds){
    this->ds_ = std::move(ds);
  }

protected:
  std::unique_ptr<deserialize_strategy> ds_;
};

class json_util_context : public i_json_util_context{
public:
  json_util_context(){
    this->ds_ = std::make_unique<nkou_deserialize_strategy>();//default for now
  }  
};



#endif //JSON_UTILS_H
