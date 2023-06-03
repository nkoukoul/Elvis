#ifndef MY_CONTROLLERS_H
#define MY_CONTROLLERS_H

#include <elvis/app_context.h>
#include <elvis/controllers.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class FileGetController : public Elvis::IController
{
public:
  FileGetController();

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class FilePostController : public Elvis::IController
{
public:
  FilePostController();

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class TriggerPostController : public Elvis::IController
{
public:
  TriggerPostController();
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class UserPostController : public Elvis::IController
{
public:
  UserPostController();
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};
#endif // MY_CONTROLLERS_H