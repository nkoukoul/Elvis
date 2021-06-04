#ifndef MY_MODELS_H
#define MY_MODELS_H

#include <elvis/app_context.h>
#include <elvis/models.h>

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

#endif //MY_MODELS_H