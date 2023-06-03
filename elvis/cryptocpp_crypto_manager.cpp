//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "crypto_manager.h"
#include <cryptopp/base64.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <iostream>

using namespace Elvis;

virtual std::string
CryptocppCryptoManager::GenerateSHA1(std::string input) const override
{
  std::string digest, output;
  CryptoPP::SHA1 hash;
  hash.Update((const byte *)input.data(), input.size());
  digest.resize(hash.DigestSize());
  hash.Final((byte *)&digest[0]);
  return digest;
}

virtual std::string CryptocppCryptoManager::DecodeBase64(
    std::string encoded_string) const override
{
  std::string decoded_string;
  CryptoPP::StringSource(
      encoded_string, true,
      new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decoded_string)));
  return decoded_string;
}

virtual std::string CryptocppCryptoManager::EncodeBase64(
    std::string payload) const override
{
  std::string encoded_payload;
  CryptoPP::StringSource(
      payload, true,
      new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded_payload)));
  return encoded_payload;
}

virtual bool CryptocppCryptoManager::SHA256VerifyDigest(
    std::string digest, std::string secret) const override
{
  try
  {
    CryptoPP::HMAC<CryptoPP::SHA256> hmac((const byte *)secret.data(),
                                          secret.size());
    const int flags = CryptoPP::HashVerificationFilter::THROW_EXCEPTION |
                      CryptoPP::HashVerificationFilter::HASH_AT_END;
    CryptoPP::StringSource(digest, true,
                           new CryptoPP::HashVerificationFilter(
                               hmac, NULL, flags)); // StringSource

    return true;
  }
  catch (const CryptoPP::Exception &e)
  {
    return false;
  }
}

virtual std::string
CryptocppCryptoManager::SHA256Sign(
    std::string digest, std::string secret) const override
{
  std::string hmac_signature;
  try
  {
    CryptoPP::HMAC<CryptoPP::SHA256> hmac((const byte *)secret.data(),
                                          secret.size());

    CryptoPP::StringSource(digest, true,
                           new CryptoPP::HashFilter(
                               hmac, new CryptoPP::StringSink(hmac_signature)));
    return hmac_signature;
  }
  catch (const CryptoPP::Exception &e)
  {
    std::cout << e.what() << "\n";
    return "";
  }
}

virtual bool
CryptocppCryptoManager::VerifySHA1(std::string password,
                                   std::string digest) const override
{
  CryptoPP::SHA1 hash;
  hash.Update((const byte *)password.data(), password.size());
  return hash.Verify((const byte *)digest.data());
}
