#include "models.h"

//model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
file_model::file_model(){
  filename_ = attribute<std::string>();
  md5sum_ =  attribute<std::string>();
}

void file_model::model_map(std::list<std::unordered_map<std::string, std::string>> && deserialized_object){
  for (auto file : deserialized_object){//now it is only one always
    filename_.set(file["filename"]);
    md5sum_.set(file["md5"]);
  }
}

void file_model::repr(){
  std::cout << "filename : " << filename_.get() << "\n";
  std::cout << "md5sum : " << md5sum_.get() << "\n";
}
