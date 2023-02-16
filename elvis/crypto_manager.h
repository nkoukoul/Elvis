//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#ifndef CRYPTO_MANAGER_H
#define CRYPTO_MANAGER_H

#include <string>

namespace Elvis
{
class ICryptoManager
{
public:
    virtual std::string GenerateSHA1(std::string input) const = 0;
    virtual std::string DecodeBase64(std::string encoded_payload) const = 0;
    virtual std::string EncodeBase64(std::string payload) const = 0;
    virtual bool SHA256VerifyDigest(std::string digest, std::string secret) const = 0;
    virtual std::string SHA256Sign(std::string digest, std::string secret) const = 0;
    virtual bool VerifySHA1(std::string password, std::string digest) const = 0;
};

class MockCryptoManager final : public ICryptoManager
{
public:
    virtual std::string GenerateSHA1(std::string input) const override
    {
        return "";
    }

    virtual std::string DecodeBase64(std::string encoded_payload) const override
    {
        return "";
    }

    virtual std::string EncodeBase64(std::string payload) const override
    {
        return "";
    }

    virtual bool SHA256VerifyDigest(std::string digest, std::string secret) const override
    {
        return true;
    }

    virtual std::string SHA256Sign(std::string digest, std::string secret) const override
    {
        return "";
    }

    virtual bool VerifySHA1(std::string password, std::string digest) const override
    {
        return true;
    }
};

class CryptocppCryptoManager final : public ICryptoManager
{
public:
    virtual std::string GenerateSHA1(std::string input) const override;

    virtual std::string DecodeBase64(std::string encoded_string) const override;

    virtual std::string EncodeBase64(std::string payload) const override;

    virtual bool SHA256VerifyDigest(std::string digest, std::string secret) const override;

    virtual std::string SHA256Sign(std::string digest, std::string secret) const override;

    virtual bool VerifySHA1(std::string password, std::string digest) const override;
};
}
#endif // CRYPTO_MANAGER_H