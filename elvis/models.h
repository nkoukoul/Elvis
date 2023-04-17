//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef MODELS_H
#define MODELS_H

#include "app_context.h"
#include <iostream>
#include <list>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class IAttribute
{
public:
  virtual ~IAttribute() = default;

  template <class T>
  T getValue();

  template <class T>
  void setValue(T value);

  virtual std::string getKey() const = 0;
  virtual void setKey(std::string key) = 0;
};

template <class T>
class Attribute : public IAttribute
{
private:
  T m_Value;
  std::string m_Key;

public:
  T getValue() { return m_Value; }

  void setValue(T value) { m_Value = value; }

  virtual std::string getKey() const override { return m_Key; }

  virtual void setKey(std::string key) override { m_Key = key; }
};

template <class T>
T IAttribute::getValue()
{
  return static_cast<Attribute<T> *>(this)->getValue();
}

template <class T>
void IAttribute::setValue(T value)
{
  static_cast<Attribute<T> *>(this)->setValue(value);
}

class IModel
{
public:
  virtual void Create(App *ac) const = 0;
  virtual void Retrieve(App *ac) const = 0;
  virtual void Display() const = 0;
};

#endif // MODELS_H
