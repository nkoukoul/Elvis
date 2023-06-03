#include "my_models.h"
#include <sstream>

using namespace Elvis;
// model is {"filename": "test.txt",  "md5sum":
// "5f7f11f4b89befa92c9451ffa5c81184"}
FileModel::FileModel(std::string filename, std::string md5)
{
  m_Filename = std::make_unique<Attribute<std::string>>();
  m_Filename->setKey("filename");
  m_Filename->setValue(filename);
  m_Md5 = std::make_unique<Attribute<std::string>>();
  m_Md5->setKey("md5sum");
  m_Md5->setValue(md5);
}

void FileModel::Create() const
{
  std::string sql = "insert into operations (";
  std::ostringstream values;
  values << "values (";
  if (dynamic_cast<Attribute<std::string> *>(m_Filename.get())->getValue() !=
      "")
  {
    sql += m_Filename->getKey();
    values << "'" +
                  dynamic_cast<Attribute<std::string> *>(m_Filename.get())
                      ->getValue() +
                  "'";
  }
  if (dynamic_cast<Attribute<std::string> *>(m_Md5.get())->getValue() != "")
  {
    sql += ", ";
    sql += m_Md5->getKey();
    values << ", ";
    values
        << "'" +
               dynamic_cast<Attribute<std::string> *>(m_Md5.get())->getValue() +
               "'";
  }
  sql += ") ";
  values << ")";
  sql += values.str();
  
  auto elvis = App::GetInstance();
  elvis->CreateModel(sql);
}

void FileModel::Retrieve() const
{
  std::string filenameValue =
      dynamic_cast<Attribute<std::string> *>(m_Filename.get())->getValue();
  std::string md5Value =
      dynamic_cast<Attribute<std::string> *>(m_Md5.get())->getValue();
  std::string sql = "select id, " + m_Filename->getKey() + ", " +
                    m_Md5->getKey() + " from operations";
  std::ostringstream values;
  values << "values (";
  if (!filenameValue.empty() && !md5Value.empty())
  {
    sql += "where ";
    values << m_Filename->getKey() << " = '" + filenameValue + "' and ";
    values << m_Md5->getKey() << " = '" + md5Value + "'";
  }
  else if (!filenameValue.empty())
  {
    sql += "where ";
    values << m_Filename->getKey() << " = '" + filenameValue + "'";
  }
  else if (!md5Value.empty())
  {
    sql += "where ";
    values << m_Md5->getKey() << " = '" + md5Value + "'";
  }
  sql += values.str();
  
  auto elvis = App::GetInstance();
  elvis->RetrieveModel(sql);
}

void FileModel::Display() const
{
  std::cout
      << m_Filename->getKey() << ": "
      << dynamic_cast<Attribute<std::string> *>(m_Filename.get())->getValue()
      << "\n";
  std::cout << m_Md5->getKey() << ": "
            << dynamic_cast<Attribute<std::string> *>(m_Md5.get())->getValue()
            << "\n";
}

// void user_model::insert_model(app *ac)
// {
//   std::string sql = "insert into users (username, password) values ('" +
//   username_.get() + "','" + password_.get() + "')"; auto db_c =
//   ac->dbm_->ExtractDBConnectorFromPool<pg_connector>(); db_c->insert(sql);
//   ac->dbm_->ReturnDBConnectorToPool<pg_connector>(std::move(db_c));
// }

// void user_model::retrieve_model(app *ac)
// {
//   std::string sql = "select id, username, password from users where username
//   = '" + username_.get() + "'"; pqxx::result query_result; auto db_c =
//   ac->dbm_->ExtractDBConnectorFromPool<pg_connector>(); db_c->select(sql,
//   query_result); for (pqxx::result::const_iterator c = query_result.begin();
//   c != query_result.end(); ++c)
//   {
//     username_.set(c[1].as<std::string>());
//     password_.set(c[2].as<std::string>());
//   }
//   ac->dbm_->ReturnDBConnectorToPool<pg_connector>(std::move(db_c));
// }

// void user_model::repr()
// {
//   std::cout << "username : " << username_.get() << "\n";
//   std::cout << "password : " << password_.get() << "\n";
// }