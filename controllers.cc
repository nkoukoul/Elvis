#include "controllers.h"


std::string file_get_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data){
  std::string page = "<!doctype html>"
    "<html>"
    "<head>"
    "<title>This is the title of the webpage!</title>"
    "</head>"
    "<body>"
    "<p>This is an example paragraph. Anything in the <strong>body</strong> tag will appear on the page, just like this <strong>p</strong> tag and its contents.</p>"
    "</body>"
    "</html>";
  return page;
}

std::string file_post_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data){

  return{};
}
