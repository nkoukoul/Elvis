#ifndef MY_CONTROLLERS_H
#define MY_CONTROLLERS_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <elvis/controllers.h>
#include <elvis/app_context.h>
#include "my_models.h"


class file_get_controller : public i_controller
{
public:
  file_get_controller() = default;

  void do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) override;
};

class file_post_controller : public i_controller
{
public:
  file_post_controller() = default;

  void do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) override;
};

class trigger_post_controller : public i_controller
{
public:
  trigger_post_controller() = default;
  void do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) override;
};

class user_post_controller : public i_controller
{
public:
  user_post_controller() = default;
  void do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) override;
};
#endif //MY_CONTROLLERS_H