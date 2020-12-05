//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//
#include "controllers.h"
#include "app_context.h"
#include "models.h"
#include <fstream>

std::string read_from_file(std::string filepath, std::string filename){
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

//route is /file/{filename}
std::string file_get_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac){
  //check for file existense should be added
  std::size_t index = deserialized_input_data["url"].find_last_of("/");
  std::string filename = deserialized_input_data["url"].substr(index+1);
  if (!ac->app_cache_->find(filename)){
    ac->app_cache_->insert(std::make_pair(filename, std::move(read_from_file("", filename))));
  }
  return (*(ac->app_cache_))[filename].second;
}

//route is /file body is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
std::string file_post_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac){
  //eg model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
  std::unique_ptr<file_model> fm = std::make_unique<file_model>();
  // this is json data so further deserialization is needed
  fm->model_map(std::move(ac->juc_->do_deserialize(std::move(deserialized_input_data["data"]))));
  fm->repr();
  ac->app_cache_->insert(std::make_pair(fm->get_filename(), std::move(read_from_file("", fm->get_filename()))));
  ac->app_cache_->state();
  return {};
}
