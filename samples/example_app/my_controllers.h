#ifndef MY_CONTROLLERS_H
#define MY_CONTROLLERS_H

#include <elvis/app_context.h>
#include <elvis/controllers.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class FileGetController : public Elvis::IController {
private:
  app *ac;

public:
  FileGetController(app *ac);

  void DoStuff(std::unordered_map<std::string, std::string>
                   &deserialized_input_data) override;
};

class FilePostController : public Elvis::IController {
private:
  app *ac;

public:
  FilePostController(app *ac);

  void DoStuff(std::unordered_map<std::string, std::string>
                   &deserialized_input_data) override;
};

class trigger_post_controller : public Elvis::IController {
private:
  app *ac;

public:
  trigger_post_controller(app *ac);
  void DoStuff(std::unordered_map<std::string, std::string>
                   &deserialized_input_data) override;
};

class user_post_controller : public Elvis::IController {
private:
  app *ac;

public:
  user_post_controller(app *ac);
  void DoStuff(std::unordered_map<std::string, std::string>
                   &deserialized_input_data) override;
};
#endif // MY_CONTROLLERS_H