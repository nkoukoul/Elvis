#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <unordered_map>
#include <string>
#include <stack>
#include <list>

class deserialize_strategy
{
public:
  deserialize_strategy() = default;
  virtual std::list<std::unordered_map<std::string, std::string>> deserialize(std::string const & input) = 0;
};


class nkou_deserialize_strategy : public deserialize_strategy
{
public:
  std::list<std::unordered_map<std::string, std::string>> deserialize(std::string const & input) override;
};


class json_util_context{
public:
  json_util_context() = default;
  
  void do_deserialize(std::string const & input){
    ds->deserialize(input);
  }
  
  void set_deserialize_strategy(){
    this->ds = new nkou_deserialize_strategy();
  }

private:
  deserialize_strategy * ds;
};



#endif //JSON_UTILS_H
