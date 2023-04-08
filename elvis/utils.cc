#include "utils.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iostream>
#include <memory>

std::string daytime_()
{
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", timeinfo);
  std::string time(buffer);

  return time;
}

unsigned int binary_to_decimal(std::string binary_num)
{
  unsigned int dec_value = 0;

  unsigned int base = 1;

  for (int i = binary_num.length() - 1; i >= 0; i--)
  {
    if (binary_num[i] == '1')
      dec_value += base;
    base = base * 2;
  }

  return dec_value;
}

std::string base_64_encode(std::string input)
{

  std::vector<char> base64_lookup = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

  std::string bstring;
  for (int i = 0; i < input.size(); i++)
  {
    std::bitset<8> bb(input[i]);
    bstring += bb.to_string();
  }

  std::string padding;

  if (bstring.size() % 24 == 16)
  {
    bstring += "00";
    padding += "=";
  }

  if (bstring.size() % 24 == 8)
  {
    bstring += "0000";
    padding += "==";
  }

  std::string output;

  for (int i = 0; i < bstring.size() / 6; i++)
  {
    std::string b_sub = bstring.substr(i * 6, 6);
    output += base64_lookup[binary_to_decimal(b_sub)];
  }
  output += padding;
  return output;
}

std::string generate_ws_key(std::string ws_client_key, std::shared_ptr<Elvis::ICryptoManager> crypto_manager)
{

  std::string magic_ws_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string concatenated_string = ws_client_key + magic_ws_string;
  auto sha1 = crypto_manager->GenerateSHA1(concatenated_string);
  auto encoded_key = crypto_manager->EncodeBase64(sha1);
  return encoded_key.substr(0, encoded_key.size() - 1);
}

std::string read_from_file(std::string filepath, std::string filename)
{
  std::streampos size;
  char *memblock;
  std::string output;
  std::ifstream file(filepath + filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open())
  {
    size = file.tellg();
    memblock = new char[(int)size + 1];
    file.seekg(0, std::ios::beg);
    file.read(memblock, size);
    file.close();
    memblock[size] = '\0';
    // cout << "the entire file content is in memory";
    output = memblock;
    delete[] memblock;
  }
  return output;
}

bool jwt_verify(std::string jwt, std::shared_ptr<Elvis::ICryptoManager> crypto_manager)
{
  std::string secret = "test1234";
  std::size_t dot_index = jwt.find_first_of(".");
  std::string encoded_header;
  if (dot_index != std::string::npos)
  {
    encoded_header = jwt.substr(0, dot_index);
  }
  else
  {
    return false;
  }
  std::string header = crypto_manager->DecodeBase64(encoded_header);
  std::size_t second_dot_index = jwt.substr(dot_index + 1).find_first_of(".");
  std::string encoded_payload;
  if (dot_index != std::string::npos)
  {
    encoded_payload = jwt.substr(dot_index + 1, second_dot_index);
  }
  else
  {
    return false;
  }
  std::string payload = crypto_manager->DecodeBase64(encoded_payload);
  std::string encoded_signature = jwt.substr(dot_index + second_dot_index + 2);
  std::string signature = encoded_header + "." + encoded_payload;
  std::string hmac_signature = crypto_manager->DecodeBase64(encoded_signature);
  return crypto_manager->SHA256VerifyDigest(signature + hmac_signature, secret);
}

std::string jwt_sign(std::string user_name, std::shared_ptr<Elvis::ICryptoManager> crypto_manager)
{
  std::string secret = "test1234";
  std::string header = R"({"alg": "HS256", "typ": "JWT"})";
  std::chrono::time_point<std::chrono::system_clock> iat = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> exp = iat + std::chrono::minutes(5);
  std::string encoded_header = crypto_manager->EncodeBase64(header);
  auto iat_t = std::chrono::system_clock::to_time_t(iat);
  auto exp_t = std::chrono::system_clock::to_time_t(exp);
  std::ostringstream os;
  os << R"({"admin": false, "name": ")" << user_name << R"(",  "iat": )" << iat_t << R"(,  "exp": )" << exp_t << "}";
  std::string payload = os.str();
  std::string encoded_payload = crypto_manager->EncodeBase64(payload);

  std::string signature = encoded_header + "." + encoded_payload;
  std::string hmac_signature = crypto_manager->SHA256Sign(signature, secret);
  std::string encoded_signature = crypto_manager->EncodeBase64(hmac_signature);
  std::string jwt = encoded_header + "." + encoded_payload + "." + encoded_signature;
  return jwt;
}

std::string generate_password_hash(std::string password, std::shared_ptr<Elvis::ICryptoManager> crypto_manager)
{
  return crypto_manager->GenerateSHA1(password);
}

bool verify_password_hash(std::string password, std::string digest, std::shared_ptr<Elvis::ICryptoManager> crypto_manager)
{
  return crypto_manager->VerifySHA1(password, digest);
}