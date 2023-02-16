//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "controllers.h"
#include "app_context.h"
#include "response_context.h"
#include "queue.h"
#include <future>

void Elvis::IController::Run(std::shared_ptr<Elvis::ClientContext> c_ctx, app *ac)
{
  DoStuff(c_ctx->m_HttpHeaders, ac);
  std::future<void> event = std::async(std::launch::deferred, &Elvis::IResponseContext::DoCreateResponse, ac->m_HttpResponseContext.get(), c_ctx);
  ac->m_AsyncQueue->CreateTask(std::move(event));
}
