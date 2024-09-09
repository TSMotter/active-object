#ifndef __IEVENT_H_
#define __IEVENT_H_

#include <vector>
#include <memory>
#include <cstdint>
#include <typeinfo>

#include <boost/signals2.hpp>

class IEvent
{
   public:
    virtual ~IEvent() = default;
    virtual std::size_t getTypeHash() const
    {
        return typeid(*this).hash_code();
    }

   protected:
    IEvent()
    {
    }
};

using IEvent_ptr      = std::shared_ptr<IEvent>;
using SignatureIEvent = std::function<void(IEvent_ptr)>;
using SignalIEvent    = boost::signals2::signal<void(IEvent_ptr)>;

#endif