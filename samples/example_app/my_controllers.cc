#include "my_controllers.h"

//route is /file/{filename}
void file_get_controller::do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac)
{
  //check for file existense should be added
  std::size_t index = deserialized_input_data["url"].find_last_of("/");
  std::string filename = deserialized_input_data["url"].substr(index + 1);
  auto fm = std::make_unique<file_model>();
  fm->filename_.set(filename);
  fm->retrieve_model(ac);
  //fm->repr();
  auto cache = ac->cm_->access_cache<t_cache<std::string, std::string>>();
  std::string controller_data = cache->operator[](filename);
  if (controller_data.empty())
  {
    controller_data = read_from_file("", filename);
    cache->insert(std::make_pair(filename, controller_data));
  }
  deserialized_input_data["controller_data"] = controller_data;
}

// route is /file body is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
// used to refresh or add to cache content
void file_post_controller::do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac)
{
  // this is json data so further deserialization is needed
  auto input = ac->juc_->do_deserialize(std::move(deserialized_input_data["data"])).front();
  //eg input data is {"filename": "test.txt",  "md5sum_": "5f7f11f4b89befa92c9451ffa5c81184"}
  auto fm = std::make_unique<file_model>(input["filename"], input["md5sum"]);
  //fm->repr();
  fm->insert_model(ac);
  auto cache = ac->cm_->access_cache<t_cache<std::string, std::string>>();
  cache->insert(std::make_pair(fm->filename_.get(),
                               std::move(read_from_file("", fm->filename_.get()))));
  cache->state();
}

// http request is used to trigger a broadcast to all the ws clients
void trigger_post_controller::do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac)
{
  // this is json data so further deserialization is needed
  std::unordered_map<std::string, std::string> input_args =
      {{"data", read_from_file("", "index.html")}, {"Connection", "open"}};
  for (auto fd : ac->broadcast_fd_list)
  {

    // executor->produce_event<std::function<void()>>(
    // std::move(
    //     std::bind(
    //         &i_response_context::do_create_response,
    //         ac->ws_ioc_->res_.get(),
    //         fd_pair.first,
    //         input_args,
    //         executor)));
  }
}

void user_post_controller::do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac)
{
  auto input = ac->juc_->do_deserialize(std::move(deserialized_input_data["data"])).front();
  //eg input data is {"username": "test",  "password": "test"}
  auto user = std::make_unique<user_model>();
  user->username_.set(input["username"]);
  user->retrieve_model(ac);
  if (verify_password_hash(input["password"], user->password_.get()))
  {
    deserialized_input_data["controller_data"] = jwt_sign(user->username_.get());
  }
}
