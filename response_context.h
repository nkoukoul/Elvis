#ifndef RESPONSE_CONTEXT_H
#define RESPONSE_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>

class response_creator{
public:
  response_creator() = default;

  virtual std::string create_response(std::string && input_data) = 0;

};

class nkou_response_creator: public response_creator{
public:
  nkou_response_creator() = default;

  std::string create_response(std::string && input_data) override;
};

class i_response_context{
public:
  i_response_context() = default;
  
  void set_response_context(std::unique_ptr<response_creator> response){
    response_ = std::move(response);
  }
  
  std::string do_create_response(std::string && input_data){
    return response_->create_response(std::move(input_data));
  }

protected:
  std::unique_ptr<response_creator> response_;
};

class http_response_context: public i_response_context{
public:
  http_response_context(){
    this->response_ = std::make_unique<nkou_response_creator>(); //default for now
  }
};

#endif //RESPONSE_CONTEXT_H