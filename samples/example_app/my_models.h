#ifndef MY_MODELS_H
#define MY_MODELS_H

#include <elvis/app_context.h>
#include <elvis/models.h>

class FileModel : IModel {
private:
  std::unique_ptr<Attribute<std::string>> m_Filename;
  std::unique_ptr<Attribute<std::string>> m_Md5;

public:
  FileModel() = delete;
  FileModel(std::string filename, std::string md5);

  virtual void Create(App *ac) const override;

  virtual void Retrieve(App *ac) const override;

  virtual void Display() const override;
};

// class user_model : public model
// {
// public:
//   user_model() = default;

//   user_model(std::string username, std::string password):username_(username),
//   password_(password){}

//   void insert_model(app *ac) override;

//   void retrieve_model(app *ac) override;

//   void repr() override;

//   attribute<std::string> username_;
//   attribute<std::string> password_;
// };

#endif // MY_MODELS_H