#ifndef MY_CONTROLLERS_H
#define MY_CONTROLLERS_H

#include <elvis/app_context.h>
#include <elvis/controllers.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class FileGetController final: public Elvis::IController
{
public:
  explicit FileGetController();

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class FilePostController final: public Elvis::IController
{
public:
  explicit FilePostController();

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class TriggerPostController final: public Elvis::IController
{
public:
  explicit TriggerPostController();
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class UserPostController final: public Elvis::IController
{
public:
  explicit UserPostController();
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};
#endif // MY_CONTROLLERS_H