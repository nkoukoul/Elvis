//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stack>
#include <list>
#include <memory>

class validate_strategy
{
public:
	validate_strategy() = default;
	virtual bool validate(std::string const& input) const = 0;
};

class elvis_validate_strategy : public validate_strategy
{
public:
	bool validate(std::string const& input) const override;
};

class deserialize_strategy
{
public:
	deserialize_strategy() = default;
	virtual std::list<std::unordered_map<std::string, std::string>> deserialize(std::string&& input) const = 0;
};

class elvis_deserialize_strategy : public deserialize_strategy
{
public:
	std::list<std::unordered_map<std::string, std::string>> deserialize(std::string&& input) const override;
};

class i_json_util_context
{
public:
	i_json_util_context() = default;

	bool do_validate(std::string const& input) const
	{
		return vs_->validate(input);
	}

	void set_validate_strategy(std::unique_ptr<validate_strategy> vs)
	{
		this->vs_ = std::move(vs);
	}

	std::list<std::unordered_map<std::string, std::string>> do_deserialize(std::string&& input) const
	{
		if (!vs_->validate(input)) return {};
		return ds_->deserialize(std::move(input));
	}

	void set_deserialize_strategy(std::unique_ptr<deserialize_strategy> ds)
	{
		this->ds_ = std::move(ds);
	}

protected:
	std::unique_ptr<deserialize_strategy> ds_;
	std::unique_ptr<validate_strategy> vs_;
};

class json_util_context : public i_json_util_context
{
public:
	json_util_context()
	{
		set_deserialize_strategy(std::make_unique<elvis_deserialize_strategy>());//default for now
		set_validate_strategy(std::make_unique<elvis_validate_strategy>());//default for now
	}
};

#endif //JSON_UTILS_H
