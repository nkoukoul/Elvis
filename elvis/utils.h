#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <fstream>
#include <bitset>
#include <vector>

std::string daytime_();
unsigned int binary_to_decimal(std::string binary_num);
std::string base_64_encode(std::string input);
std::string generate_ws_key(std::string ws_client_key);
std::string read_from_file(std::string filepath, std::string filename);
std::string jwt_sign(std::string user_name);
bool jwt_verify(std::string jwt);
std::string generate_password_hash(std::string password);
bool verify_password_hash(std::string password, std::string digest);

#endif //UTILS_H
