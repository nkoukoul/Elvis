#ifndef HTTP_CONTEXT_H
#define HTTP_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>

class request_parser{
public:
  request_parser() = default;

  virtual std::unordered_map<std::string, std::string> parse(std::string && input_data) = 0;

};

class nkou_request_parser: public request_parser{
public:
  nkou_request_parser() = default;

  std::unordered_map<std::string, std::string> parse(std::string && input_data) override;
};

class i_request_context{
public:
  i_request_context() = default;
  
  void set_request_context(std::unique_ptr<request_parser> rq){
    rq_ = std::move(rq);
  }
  
  virtual std::unordered_map<std::string, std::string>  do_parse(std::string && input_data){
    return {};
  }

protected:
  std::unique_ptr<request_parser> rq_;
};

class http_context: public i_request_context{
public:
  http_context(){
    this->rq_ = std::make_unique<nkou_request_parser>(); //default for now
  }

  std::unordered_map<std::string, std::string> do_parse(std::string && input_data){
    return rq_->parse(std::move(input_data));
  }
};

#endif //HTTP_CONTEXT_H
