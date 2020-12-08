#include "utils.h"

std::string utils::daytime_(){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  
  strftime(buffer,80,"%a %b %d %H:%M:%S %Y",timeinfo);
  std::string time(buffer);

  return time;
}

unsigned int utils::binary_to_decimal(std::string binary_num){
  unsigned int dec_value = 0;
 
  unsigned int base = 1;
 
  for (int i = binary_num.length() - 1; i >= 0; i--) {
    if (binary_num[i] == '1')
      dec_value += base;
    base = base * 2;
  }
    
  return dec_value;
}

std::string utils::base_64_encode(std::string input){
  
  std::vector<char> base64_lookup = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };
  
  std::string bstring;
  for (int i = 0; i < input.size(); i++){
    std::bitset<8> bb(input[i]);
    bstring += bb.to_string();
  }
  
  std::string padding;

  if (bstring.size()%24 == 16){
    bstring += "00";
    padding += "=";
  }

  if (bstring.size()%24 == 8){
    bstring += "0000";
    padding += "==";
  }

  std::string output;
  
  for (int i = 0; i < bstring.size()/6; i++){
    std::string b_sub = bstring.substr(i * 6, 6);
    output += base64_lookup[binary_to_decimal(b_sub)];
  }
  output += padding;
  return output;
}

std::string utils::generate_ws_key(std::string ws_client_key){
  
  std::string magic_ws_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string concatenated_string = ws_client_key + magic_ws_string;

  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)concatenated_string.c_str(), concatenated_string.size(), hash);
  std::string input;
  for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
    input += (char)hash[i];
  }
  return base_64_encode(input);
}

std::string utils::read_from_file(std::string filepath, std::string filename){
  std::streampos size;
  char * memblock;
  std::string output;
  std::ifstream file (filepath+filename, std::ios::in|std::ios::binary|std::ios::ate);
  if (file.is_open())
  {
    size = file.tellg();
    memblock = new char[(int)size+1];
    file.seekg (0, std::ios::beg);
    file.read (memblock, size);
    file.close();
    memblock[size] = '\0';
    //cout << "the entire file content is in memory";
    output = memblock;
    delete[] memblock;
  }
  return output;
}
