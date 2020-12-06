#ifndef UTILS_H
#define UTILS_H

#include <string>

class utils{
public:
  std::string daytime_();
  unsigned int binary_to_decimal(std::string binary_num);
  std::string base_64_encode(std::string input);
  std::string generate_ws_key(std::string ws_client_key);
};

#endif //UTILS_H
