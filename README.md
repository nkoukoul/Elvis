### Elvis
#### Description
Elvis intends to be a c++ web framework.  
Inside elvis folder is the library source files and headers.
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

#### To run app
./samples/example_app/server hostname port threads  

#### Example usage
Create app  
```
#include <elvis/app_context.h>

app* my_app = app::get_instance();
```
Create routes
```
std::unique_ptr<Elvis::RouteManager> routeManager = std::make_unique<Elvis::RouteManager>();
	routeManager->SetRoute("/file", "GET", std::move(std::make_unique<FileGetController>()));
	routeManager->SetRoute("/file", "POST", std::move(std::make_unique<FilePostController>()));
```

Configure app with default JSON, HTTP Req/Res and WS Req/Res context. One can also override the respective abstract classes and provide his own.
```
my_app->configure(std::move(tcp_server),
		std::move(std::make_unique<Elvis::HttpRequestContext>(my_app)),
		std::move(std::make_unique<Elvis::HttpResponseContext>(my_app)),
		std::move(std::make_unique<Elvis::WebsocketRequestContext>(my_app)),
		std::move(std::make_unique<Elvis::WebsocketResponseContext>(my_app)),
		std::move(std::make_unique<Elvis::JSONContext>()),
		std::move(routeManager));
```

Create models

```
class FileModel : IModel
{
private:
  std::unique_ptr<Attribute<std::string>> m_Filename;
  std::unique_ptr<Attribute<std::string>> m_Md5;
public:
  FileModel() = delete;
  FileModel(std::string filename, std::string md5);

  virtual void Create(app *ac) const override;

  virtual void Retrieve(app *ac) const override;

  virtual void Display() const override;
};
```

Use dbEgnine to run queries from the models
```
void FileModel::Create(app *ac) const
{
  std::string sql = "insert into operations (filename, md5) values ('test.txt', '1234')";
  ac->dbEngine->CreateModel(sql);
}
```