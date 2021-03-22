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
#include <vector>
#include <list>
#include <iostream>
#include "app_context.h"

template <class T>
class attribute
{
public:
  attribute() = default;
  attribute(T attribute) : attribute_(attribute) {}
  void set(const T attribute) { attribute_ = attribute; }
  T get() const { return attribute_; }

private:
  T attribute_;
};

class model
{
public:
  model() = default;

  virtual void insert_model(app *ac) = 0;

  virtual void retrieve_model(app *ac) = 0;

  virtual void repr() = 0;
};

class file_model : public model
{
public:
  file_model() = default;

  file_model(std::string filename, std::string md5sum):filename_(filename), md5sum_(md5sum){}
  void insert_model(app *ac) override;

  void retrieve_model(app *ac) override;

  void repr() override;

  attribute<std::string> filename_;
  attribute<std::string> md5sum_;
};

class user_model : public model
{
public:
  user_model() = default;

  user_model(std::string username, std::string password):username_(username), password_(password){}

  void insert_model(app *ac) override;

  void retrieve_model(app *ac) override;

  void repr() override;

  attribute<std::string> username_;
  attribute<std::string> password_;
};

#endif //MODELS_H
