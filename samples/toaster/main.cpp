#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>

#include <boost/signals2.hpp>
#include <boost/bind/bind.hpp>

#include "Logger/Logger.hpp"
#include "ThreadSafeQueue/ThreadSafeQueue.hpp"
#include "IState/IState.hpp"
#include "StateManager/StateManager.hpp"

#define LOG_MAIN (LOG("main.cpp", LEVEL_INFO))
#define DELAY 200

namespace Evts
{

class EventBlue : public IEvent
{
   public:
    int timeout;
};
class EventGreen : public IEvent
{
   public:
    std::vector<int> data;
};
class EventShutdown : public IEvent
{
};

}  // namespace Evts

/* ============================================================================================== */

namespace Toaster
{
enum class StateValue
{
    UNKNOWN = 0,
    ROOT,
    HEATING,
    DOOR_OPEN,
    TOASTING,
    BAKING
};

// Forward Declaration
class Toaster;

class ToasterSuperState : public IState<Toaster>
{
   public:
    virtual ~ToasterSuperState() = default;
    ToasterSuperState(Toaster* act) : IState<Toaster>(act)
    {
    }

    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class Heating : public ToasterSuperState
{
   public:
    virtual ~Heating() = default;
    Heating(Toaster* actor)
        : ToasterSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};
class DoorOpen : public ToasterSuperState
{
   public:
    virtual ~DoorOpen() = default;
    DoorOpen(Toaster* actor)
        : ToasterSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};
class Toasting : public Heating
{
   public:
    virtual ~Toasting() = default;
    Toasting(Toaster* actor)
        : Heating(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};
class Baking : public Heating
{
   public:
    virtual ~Baking() = default;
    Baking(Toaster* actor)
        : Heating(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class Toaster
{
   public:
    using ToasterSuperState_ptr = std::shared_ptr<ToasterSuperState>;
    Toaster() : m_queue{std::make_shared<SimplestThreadSafeQueue<IEvent_ptr>>()}
    {

    }
    ~Toaster()
    {
    }
};

}

/* ============================================================================================== */

int main(int argc, char** argv)
{

}