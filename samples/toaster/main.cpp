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

class DoorOpen : public IEvent
{
};
class DoorClose : public IEvent
{
};
class DoToasting : public IEvent
{
   public:
    // DoToasting() : timeout{60000}
    //{
    // }
    uint64_t timeout;
};
class DoBaking : public IEvent
{
   public:
    // DoBaking() : temperature{140}
    //{
    // }
    float temperature;
};
class Timeout : public IEvent
{
};
class Shutdown : public IEvent
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
    Heating(Toaster* actor) : ToasterSuperState(actor)
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
    DoorOpen(Toaster* actor) : ToasterSuperState(actor)
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
    Toasting(Toaster* actor) : Heating(actor)
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
    Baking(Toaster* actor) : Heating(actor)
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
        tree<ToasterSuperState_ptr> tree;
        tree.set_head(std::make_shared<ToasterSuperState>(this));
        m_states[StateValue::ROOT] = tree.begin();
        m_states[StateValue::HEATING] =
            tree.append_child(m_states[StateValue::ROOT], std::make_shared<Heating>(this));
        m_states[StateValue::DOOR_OPEN] =
            tree.append_child(m_states[StateValue::ROOT], std::make_shared<DoorOpen>(this));
        m_states[StateValue::TOASTING] =
            tree.append_child(m_states[StateValue::HEATING], std::make_shared<Toasting>(this));
        m_states[StateValue::BAKING] =
            tree.append_child(m_states[StateValue::HEATING], std::make_shared<Baking>(this));
        m_states[StateValue::UNKNOWN] = tree.end();

        m_next_state = m_states[StateValue::UNKNOWN];

        m_state_manager = std::make_shared<StateManager<ToasterSuperState_ptr>>(
            std::move(tree), m_states[StateValue::HEATING]);
    }
    ~Toaster()
    {
    }

    void start()
    {
        m_state_manager->init();
        m_running = true;
        m_thread  = std::thread(&Toaster::run, this);
    }

    void stop()
    {
        if (!m_running)
            return;

        m_running = false;
        m_queue->clear();

        if (m_thread.joinable())
            m_thread.join();
    };

    void callback_IEvent(IEvent_ptr event)
    {
        m_queue->put(event);
    }

    template <class T>
    connection connect_callbacks(T&& handler)
    {
        return m_signal.connect(handler);
    }

    void heater_on()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void heater_off()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void arm_time_event()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void disarm_time_event()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void set_temperature()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void reset_temperature()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void internal_lamp_on()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    void internal_lamp_off()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }
    void run_once()
    {
        m_running = false;
        run();
    }

    SignalIEvent                                                m_signal;
    std::shared_ptr<StateManager<ToasterSuperState_ptr>>        m_state_manager;
    std::map<StateValue, tree<ToasterSuperState_ptr>::iterator> m_states;
    tree<ToasterSuperState_ptr>::iterator                       m_next_state;

   private:
    void run()
    {
        do
        {
            IEvent_ptr current_event = m_queue->wait_and_pop();
            m_state_manager->processEvent(current_event);

            if (m_next_state != m_states[StateValue::UNKNOWN])
            {
                m_state_manager->transitionTo(m_next_state);
                m_next_state = m_states[StateValue::UNKNOWN];
            }

        } while (m_running);
    };

    std::atomic_bool                                     m_running{false};
    std::thread                                          m_thread;
    std::shared_ptr<SimplestThreadSafeQueue<IEvent_ptr>> m_queue;
};

int ToasterSuperState::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int ToasterSuperState::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int ToasterSuperState::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int Heating::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->heater_on();
    return 0;
}
int Heating::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->heater_off();
    return 0;
}
int Heating::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;

    std::size_t event_type = event->getTypeHash();
    if (event_type == typeid(Evts::DoorOpen).hash_code())
    {
        m_actor->m_next_state = m_actor->m_states[StateValue::DOOR_OPEN];
        return 0;
    }
    else if (event_type == typeid(Evts::DoToasting).hash_code())
    {
        m_actor->m_next_state = m_actor->m_states[StateValue::TOASTING];
        return 0;
    }
    else if (event_type == typeid(Evts::DoBaking).hash_code())
    {
        m_actor->m_next_state = m_actor->m_states[StateValue::BAKING];
        return 0;
    }
    return -1;  // unhandled event
}
int DoorOpen::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->internal_lamp_on();
    return 0;
}
int DoorOpen::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->internal_lamp_off();
    return 0;
}
int DoorOpen::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;

    std::size_t event_type = event->getTypeHash();
    if (event_type == typeid(Evts::DoorClose).hash_code())
    {
        m_actor->m_next_state = m_actor->m_states[StateValue::HEATING];
        return 0;
    }
    return -1;  // unhandled event
}
int Toasting::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->arm_time_event();
    return 0;
}
int Toasting::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->disarm_time_event();
    return 0;
}
int Toasting::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;

    std::size_t event_type = event->getTypeHash();
    if (event_type == typeid(Evts::Timeout).hash_code())
    {
        m_actor->m_next_state = m_actor->m_states[StateValue::HEATING];
        return 0;
    }
    return -1;  // unhandled event
}
int Baking::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->set_temperature();
    return 0;
}
int Baking::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    m_actor->reset_temperature();
    return 0;
}
int Baking::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
}  // namespace Toaster

/* ============================================================================================== */

int main(int argc, char** argv)
{
    std::vector<IEvent_ptr> events;
    events.emplace_back(std::make_shared<Evts::DoorOpen>());    // 0
    events.emplace_back(std::make_shared<Evts::DoorClose>());   // 1
    events.emplace_back(std::make_shared<Evts::DoToasting>());  // 2
    events.emplace_back(std::make_shared<Evts::DoBaking>());    // 3
    events.emplace_back(std::make_shared<Evts::Timeout>());     // 4
    events.emplace_back(std::make_shared<Evts::Shutdown>());    // 5

    std::shared_ptr<Toaster::Toaster> tst = std::make_shared<Toaster::Toaster>();
    // tst->connect_callbacks();
    // tst->start();
    tst->m_state_manager->init();

    int choice;
    while (true)
    {
        std::cout << "Enter event number: ";
        std::cin >> choice;

        if (choice == -1)
            break;

        if (choice >= 0 && choice < events.size())
        {
            tst->callback_IEvent(events[choice]);
            tst->run_once();
        }
        else
        {
            std::cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}