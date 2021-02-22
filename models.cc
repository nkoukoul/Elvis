//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "models.h"

//model is {"filename": "test.txt",  "md5sum": "5f7f11f4b89befa92c9451ffa5c81184"}
void file_model::insert_model(app *ac)
{
  std::string sql = "insert into operations (filename, md5sum) values ('" 
    + filename_.get() + "','" + md5sum_.get() + "')";
  auto db_c = ac->dbm_->access_db_connector();
  db_c->insert(sql);
  ac->dbm_->return_db_connector(std::move(db_c));
}

void file_model::retrieve_model(app *ac)
{
  std::string sql = "select id, filename, md5sum from operations where filename = '" + filename_.get() + "'";
  pqxx::result query_result;
  auto db_c = ac->dbm_->access_db_connector();
  db_c->select(sql, query_result);
  for (pqxx::result::const_iterator c = query_result.begin(); c != query_result.end(); ++c)
  {
    filename_.set(c[1].as<std::string>());
    md5sum_.set(c[2].as<std::string>());
  }
  ac->dbm_->return_db_connector(std::move(db_c));
}

void file_model::repr()
{
  std::cout << "filename : " << filename_.get() << "\n";
  std::cout << "md5sum : " << md5sum_.get() << "\n";
}
