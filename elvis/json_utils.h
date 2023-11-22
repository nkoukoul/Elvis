//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Elvis
{
class IValidateStrategy
{
public:
    virtual ~IValidateStrategy() = default;
    virtual bool Validate(std::string const& input) const = 0;
};

class ValidateStrategy final : public IValidateStrategy
{
public:
    virtual bool Validate(std::string const& input) const override;
};

class IDeserializeStrategy
{
public:
    virtual ~IDeserializeStrategy() = default;
    virtual std::list<std::unordered_map<std::string, std::string>> Deserialize(std::string&& input) const = 0;
};

class DeserializeStrategy final : public IDeserializeStrategy
{
public:
    std::list<std::unordered_map<std::string, std::string>> Deserialize(std::string&& input) const override;
};

class IJSONContext
{
public:
    virtual ~IJSONContext() = default;

    bool DoValidate(std::string const& input) const
    {
        return m_ValidateStrategy->Validate(input);
    }

    void setValidateStrategy(std::unique_ptr<IValidateStrategy> validateStrategy)
    {
        this->m_ValidateStrategy = std::move(validateStrategy);
    }

    std::list<std::unordered_map<std::string, std::string>> DoDeserialize(std::string&& input) const
    {
        if (!m_ValidateStrategy->Validate(input))
        {
            return {};
        }
        return m_DeserializeStrategy->Deserialize(std::move(input));
    }

    void setDeserializeStrategy(std::unique_ptr<IDeserializeStrategy> deserializeStrategy)
    {
        this->m_DeserializeStrategy = std::move(deserializeStrategy);
    }

protected:
    std::unique_ptr<IDeserializeStrategy> m_DeserializeStrategy;
    std::unique_ptr<IValidateStrategy> m_ValidateStrategy;
};

class JSONContext final : public IJSONContext
{
public:
    JSONContext()
    {
        setDeserializeStrategy(std::make_unique<DeserializeStrategy>()); // default for now
        setValidateStrategy(std::make_unique<ValidateStrategy>());       // default for now
    }
};
} // namespace Elvis

#endif // JSON_UTILS_H
