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
private:
  App *ac;

public:
  FileGetController(App *ac);

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class FilePostController : public Elvis::IController
{
private:
  App *ac;

public:
  FilePostController(App *ac);

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class TriggerPostController : public Elvis::IController
{
private:
  App *ac;

public:
  TriggerPostController(App *ac);
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

class UserPostController : public Elvis::IController
{
private:
  App *ac;

public:
  UserPostController(App *ac);
  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};
#endif // MY_CONTROLLERS_H