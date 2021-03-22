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

#endif //MODELS_H
