#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <unordered_map>
#include <string>

class i_controller{
public:
  i_controller() = default;
  virtual std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data) = 0;
};

class file_get_controller: public i_controller{
public:
  file_get_controller() = default;
  std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data) override;
};

class file_post_controller: public i_controller{
public:
  file_post_controller() = default;
  std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data) override;
};

#endif //CONTROLLERS_H
