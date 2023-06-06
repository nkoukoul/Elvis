### Elvis
#### Description
Elvis intends to be a c++ web framework.  
Inside elvis folder are the library source files and headers.
Inside samples folder there is an example application.
CMAKE support has been added. Two third party libraries are used,
libpqxx version 5.0.1 and cryptopp version 5.6.2
#### To build without postgres and crypto support (no websocket):
1. install cpp devtools and cmake
2. cmake ./
3. make -> will generate a static Elvis.a lib and link with samples/example_app
#### To build with crypto support (websocket supported):
1. install cpp devtools and cmake
2. cmake -DUSE_CRYPTO=ON ./
3. make -> will generate a static Elvis.a lib and link with samples/example_app
#### To build with postgres and crypto support (websocket supported):  
1. install cpp devtools and cmake
2. cmake -DUSE_POSTGRES=ON -DUSE_CRYPTO=ON ./
3. make -> will generate a static Elvis.a lib and link with samples/example_app
#### Other cmake options supported:
1. -DDEBUG -> Debug build with connection monitor enabled.
2. -DCONSOLE -> Log everything on stdout.

#### To run example app
./samples/example_app/server hostname port threads  

### Example usage
Create and access elvis from everywhere.  
```
#include <elvis/app_context.h>

app* elvis = Elvis::App::GetInstance();
```

#### API to access internals
Use framework cache to cache data:
```
std::string key;
std::string data;
elvis->Cache(key, data);
```

Use framework cache to retrieve data:
```
std::string key;
std::string data = elvis->GetCacheData(key);
```

Use framework JSON deserializer:
```
std::string serializedData;
std::list<std::unordered_map<std::string, std::string>> json = elvis->JSONDeserialize(std::move(serializedData));
```
Use framework DB engine to create and retrieve models based on string query.
```
std::string query;
elvis->CreateModel(query);
elvis->RetrieveModel(query);
```
#### Create routes
```
std::unique_ptr<Elvis::RouteManager> routeManager = std::make_unique<Elvis::RouteManager>();
	routeManager->SetRoute("/file", "GET", std::move(std::make_unique<FileGetController>()));
	routeManager->SetRoute("/file", "POST", std::move(std::make_unique<FilePostController>()));
```

Configure app with hostname, port, thread number, routes logfile and loglevel.
```
elvis->Configure(ipaddr, port, thread_number, routeManager, "server.log", Elvis::LogLevel::INFO);
```
Default configure provides
* IOContext (elvis/io_context.h)
* Logger (elvis/logger.h)
* Concurrent Queue with thread pooling (elvis/queue.h)
* CryptoManager utils (elvis/crypto_manager.h, elvis/utils.h)
* Http Request/Response Context (elvis/request_context.h, elvis/response_context.h)
* WS Request/Response Context (if a crypto library is present) (elvis/request_context.h, elvis/response_context.h)
* JSON parsing and validating utils (elvis/json_utils.h)
* LRU Cache (there is also a timer based cash) (elvis/cache.h)
* Monitor for incoming connections available for debug builds (elvis/monitor.h)

Run threaded applications. The call below will block.
```
elvis->Run();
```

#### Create models and define the CRUD api (here only Create and Retrieve are implemented, display serializes model to string for display).
```
// Override IModel
class FileModel final: IModel {
private:
  std::unique_ptr<Attribute<std::string>> m_Filename;
  std::unique_ptr<Attribute<std::string>> m_Md5;

public:
  FileModel() = delete;
  explicit FileModel(std::string filename, std::string md5);

  virtual void Create() const override;

  virtual void Retrieve() const override;

  virtual void Display() const override;
};

void FileModel::Create() const
{
  std::string sql = "insert into operations (filename, md5) values ('test.txt', '1234')";
  auto elvis = App::GetInstance();
  elvis->CreateModel(sql);
}
```

#### Endpoint Example
```
// Override Elvis::IController
class FilePostController final: public Elvis::IController
{
public:
  explicit FilePostController();

  void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data) override;
};

void FilePostController::DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data)
{
  // This is json data so further deserialization is needed. The result is a list of objects.
  // Here we expect only one.
  auto elvis = App::GetInstance();
  auto input = elvis->JSONDeserialize(std::move(deserialized_input_data["data"])).front();
  if (input.empty())
  {
    std::cout << "FilePostController: 400 Bad Request\n";
    deserialized_input_data["status"] = "400 Bad Request";
    return;
  }
  auto file_model = std::make_unique<FileModel>(input["filename"], input["md5sum"]);
  file_model->Create();
  elvis->Cache(input["filename"], read_from_file("", input["filename"]));
}

// Register the controller.
std::unique_ptr<Elvis::RouteManager> routeManager = std::make_unique<Elvis::RouteManager>();
routeManager->SetRoute("/file", "POST", std::move(std::make_unique<FilePostController>()));
```