#ifndef CONTEXT_DELEGATE_H
#define CONTEXT_DELEGATE_H

#include <memory>

namespace Elvis
{
class InputContextDelegate
{
public:
    virtual ~InputContextDelegate() = default;
    virtual void Read(std::shared_ptr<ClientContext> c_ctx) = 0;
};

class OutputContextDelegate
{
public:
    virtual ~OutputContextDelegate() = default;
    virtual void DidWrite(std::shared_ptr<ClientContext> c_ctx) = 0;
};
} // namespace Elvis
#endif