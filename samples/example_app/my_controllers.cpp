#include "my_controllers.h"
#include "my_models.h"

// route is /file/{filename}
FileGetController::FileGetController(App *ac_) : ac(ac_) {}

void FileGetController::DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data)
{
  std::size_t index = deserialized_input_data["url"].find_last_of("/");
  std::string filename = deserialized_input_data["url"].substr(index + 1);
  // auto fm = std::make_unique<file_model>();
  // fm->filename_.set(filename);
  // fm->retrieve_model(ac);
  // fm->repr();
  std::string controller_data = ac->GetCacheData(filename);
  if (controller_data.empty())
  {
    controller_data = read_from_file("", filename);
    ac->Cache(filename, controller_data);
  }
  deserialized_input_data["controller_data"] = controller_data;
}

FilePostController::FilePostController(App *ac_) : ac(ac_) {}
// route is /file body is {"filename": "test.txt",  "md5":
// "5f7f11f4b89befa92c9451ffa5c81184"} used to refresh or add to cache content
void FilePostController::DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data)
{
  // This is json data so further deserialization is needed. The result is a list of objects.
  // Here we expect only one.
  auto input = ac->JSONDeserialize(std::move(deserialized_input_data["data"])).front();
  if (input.empty())
  {
    std::cout << "FilePostController: 400 Bad Request\n";
    deserialized_input_data["status"] = "400 Bad Request";
    return;
  }
  auto file_model = std::make_unique<FileModel>(input["filename"], input["md5sum"]);
  file_model->Create(ac);
  ac->Cache(input["filename"], read_from_file("", input["filename"]));
  // cache->State();
}

TriggerPostController::TriggerPostController(App *ac_) : ac(ac_) {}
// http request is used to trigger a broadcast to all the ws clients
void TriggerPostController::DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data)
{
  // this is json data so further deserialization is needed
  std::unordered_map<std::string, std::string> input_args = {
      {"data", read_from_file("", "index.html")}, {"Connection", "open"}};
  for (auto fd : ac->broadcast_fd_list)
  {

    // executor->CreateTask<std::function<void()>>(
    // std::move(
    //     std::bind(
    //         &i_response_context::do_create_response,
    //         ac->ws_ioc_->res_.get(),
    //         fd_pair.first,
    //         input_args,
    //         executor)));
  }
}

UserPostController::UserPostController(App *ac_) : ac(ac_) {}
void UserPostController::DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data)
{
  // auto input =
  // ac->m_JSONContext->DoDeserialize(std::move(deserialized_input_data["data"])).front();
  // //eg input data is {"username": "test",  "password": "test"}
  // auto user = std::make_unique<user_model>();
  // user->username_.set(input["username"]);
  // user->retrieve_model(ac);
  // if (verify_password_hash(input["password"], user->password_.get()))
  // {
  // 	deserialized_input_data["controller_data"] =
  // jwt_sign(user->username_.get());
  // }
}
