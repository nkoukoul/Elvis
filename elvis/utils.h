#ifndef UTILS_H
#define UTILS_H

#include "crypto_manager.h"
#include <bitset>
#include <fstream>
#include <string>
#include <vector>

std::string daytime_();
unsigned int binary_to_decimal(std::string binary_num);
std::string base_64_encode(std::string input);
std::string
generate_ws_key(std::string ws_client_key,
                std::shared_ptr<Elvis::ICryptoManager> crypto_manager);
std::string read_from_file(std::string filepath, std::string filename);
std::string jwt_sign(std::string user_name,
                     std::shared_ptr<Elvis::ICryptoManager> crypto_manager);
bool jwt_verify(std::string jwt,
                std::shared_ptr<Elvis::ICryptoManager> crypto_manager);
std::string
generate_password_hash(std::string password,
                       std::shared_ptr<Elvis::ICryptoManager> crypto_manager);
bool verify_password_hash(
    std::string password, std::string digest,
    std::shared_ptr<Elvis::ICryptoManager> crypto_manager);

#endif // UTILS_H
