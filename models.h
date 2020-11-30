//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef MODELS_H
#define MODELS_H

#include <unordered_map>
#include <string>
#include <stack>
#include <list>
#include <iostream>

template<class T>
class attribute
{
public:
  attribute() = default;
  attribute(T attribute):attribute_(attribute){}
  void set(const T attribute){attribute_ = attribute;}
  T get() const {return attribute_;}
private:
  T attribute_;
};

class model{
public:
  model() = default;
  virtual void model_map(std::list<std::unordered_map<std::string, std::string>> && deserialized_object) = 0;
  virtual void repr() = 0;
};

class file_model : public model{
public:
  file_model();
  file_model(attribute<std::string> filename, attribute<std::string> md5sum):filename_(filename), md5sum_(md5sum){};
  void model_map(std::list<std::unordered_map<std::string, std::string>> && deserialized_object) override;
  void repr() override;
  std::string get_filename();
  std::string get_md5sum();
private:
  attribute<std::string> filename_;
  attribute<std::string> md5sum_;
};


#endif //MODELS_H
