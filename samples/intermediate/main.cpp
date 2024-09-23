#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

namespace Foo
{
/* clang-format off */

enum class StateValue
{
    UNKNOWN = 0, ROOT, STATE_A,
    STATE_B, STATE_C, STATE_D,
    STATE_E, STATE_F, STATE_G,
};

class ActorFoo;

class ActorFooSuperState : public IState<ActorFoo>
{
   public:
    virtual ~ActorFooSuperState() = default;
    ActorFooSuperState(ActorFoo* act) : IState<ActorFoo>(act)
    {
    }

    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateA : public ActorFooSuperState
{
   public:
    virtual ~StateA() = default;
    StateA(ActorFoo* actor)
        : ActorFooSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateC : public StateA
{
   public:
    virtual ~StateC() = default;
    StateC(ActorFoo* actor)
        : StateA(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateD : public StateA
{
   public:
    virtual ~StateD() = default;
    StateD(ActorFoo* actor)
        : StateA(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateF : public StateD
{
   public:
    virtual ~StateF() = default;
    StateF(ActorFoo* actor)
        : StateD(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateG : public StateF
{
   public:
    virtual ~StateG() = default;
    StateG(ActorFoo* actor)
        : StateF(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateB : public ActorFooSuperState
{
   public:
    virtual ~StateB() = default;
    StateB(ActorFoo* actor)
        : ActorFooSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class StateE : public StateB
{
   public:
    virtual ~StateE() = default;
    StateE(ActorFoo* actor)
        : StateB(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

/* clang-format on */
class ActorFoo
{
   public:
    using ActorFooSuperState_ptr = std::shared_ptr<ActorFooSuperState>;
    ActorFoo() : m_queue{std::make_shared<SimplestThreadSafeQueue<IEvent_ptr>>()}
    {
        /* clang-format off */
        tree<ActorFooSuperState_ptr> tree;
        tree.set_head(std::make_shared<ActorFooSuperState>(this));
        m_states[StateValue::ROOT] = tree.begin();
        m_states[StateValue::STATE_A] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<StateA>(this));
        m_states[StateValue::STATE_C] = tree.append_child(m_states[StateValue::STATE_A], std::make_shared<StateC>(this));
        m_states[StateValue::STATE_D] = tree.append_child(m_states[StateValue::STATE_A], std::make_shared<StateD>(this));
        m_states[StateValue::STATE_F] = tree.append_child(m_states[StateValue::STATE_D], std::make_shared<StateF>(this));
        m_states[StateValue::STATE_G] = tree.append_child(m_states[StateValue::STATE_F], std::make_shared<StateG>(this));
        m_states[StateValue::STATE_B] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<StateB>(this));
        m_states[StateValue::STATE_E] = tree.append_child(m_states[StateValue::STATE_B], std::make_shared<StateE>(this));
        m_states[StateValue::UNKNOWN] = tree.end();

        m_next_state = m_states[StateValue::UNKNOWN];

        m_state_manager = std::make_shared<StateManager<ActorFooSuperState_ptr>>(std::move(tree), m_states[StateValue::STATE_A]);
        /* clang-format on */
    }
    ~ActorFoo()
    {
    }

    void start()
    {
        m_state_manager->init();
        m_running = true;
        m_thread  = std::thread(&ActorFoo::run, this);
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

    SignalIEvent m_signal;

    std::shared_ptr<StateManager<ActorFooSuperState_ptr>>        m_state_manager;
    std::map<StateValue, tree<ActorFooSuperState_ptr>::iterator> m_states;
    tree<ActorFooSuperState_ptr>::iterator                       m_next_state;

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

    std::atomic_bool m_running{false};
    std::thread      m_thread;

    std::shared_ptr<SimplestThreadSafeQueue<IEvent_ptr>> m_queue;
};

/* clang-format off */
int ActorFooSuperState::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int ActorFooSuperState::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int ActorFooSuperState::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int StateA::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 11;
    m_actor->m_signal(event);
    return 0;
}
int StateA::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventGreen).hash_code())
    {
        auto event_green = std::dynamic_pointer_cast<Evts::EventGreen>(event);

        if (event_green)
        {
            std::ostringstream logStream;
            logStream << "This is the event data: ";
            for (auto k : event_green->data)
            {
                logStream << k << " ";
            }
            LOG_MAIN << logStream.str() << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_G];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    else if (event_type == typeid(Evts::EventBlue).hash_code())
    {
        // Do stuff
        return 0;
    }
    return -1;  // Unhandled event
}
int StateA::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateC::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateC::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateC::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int StateD::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateD::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateD::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int StateF::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateF::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateF::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int StateG::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 22;
    m_actor->m_signal(event);
    return 0;
}
int StateG::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int StateG::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventGreen).hash_code())
    {
        auto event_green = std::dynamic_pointer_cast<Evts::EventGreen>(event);

        if (event_green)
        {
            std::ostringstream logStream;
            logStream << "This is the event data: ";
            for (auto k : event_green->data)
            {
                logStream << k << " ";
            }
            LOG_MAIN << logStream.str() << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_E];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    return -1;  // Unhandled event
}
int StateB::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int StateB::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int StateB::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int StateE::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 33;
    m_actor->m_signal(event);
    return 0;
}
int StateE::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int StateE::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventGreen).hash_code())
    {
        auto event_green = std::dynamic_pointer_cast<Evts::EventGreen>(event);

        if (event_green)
        {
            std::ostringstream logStream;
            logStream << "This is the event data: ";
            for (auto k : event_green->data)
            {
                logStream << k << " ";
            }
            LOG_MAIN << logStream.str() << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_A];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    return -1;  // Unhandled event
}

/* clang-format on */
}  // namespace Foo

/* ============================================================================================== */

namespace Bar
{
/* clang-format off */
enum class StateValue
{
    UNKNOWN = 0, ROOT, STATE_1,
    STATE_2, STATE_3
};

class ActorBar;

class ActorBarSuperState : public IState<ActorBar>
{
   public:
    virtual ~ActorBarSuperState() = default;
    ActorBarSuperState(ActorBar* act) : IState<ActorBar>(act)
    {
    }

    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class State1 : public ActorBarSuperState
{
   public:
    virtual ~State1() = default;
    State1(ActorBar* actor)
        : ActorBarSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class State3 : public State1
{
   public:
    virtual ~State3() = default;
    State3(ActorBar* actor)
        : State1(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};

class State2 : public ActorBarSuperState
{
   public:
    virtual ~State2() = default;
    State2(ActorBar* actor)
        : ActorBarSuperState(actor)
    {
    }
    virtual int on_entry() override;
    virtual int on_exit() override;
    virtual int process_event(IEvent_ptr event) override;
};


/* clang-format on */
class ActorBar
{
   public:
    using ActorBarSuperState_ptr = std::shared_ptr<ActorBarSuperState>;
    ActorBar() : m_queue{std::make_shared<SimplestThreadSafeQueue<IEvent_ptr>>()}
    {
        /* clang-format off */
        tree<ActorBarSuperState_ptr> tree;
        tree.set_head(std::make_shared<ActorBarSuperState>(this));
        m_states[StateValue::ROOT] = tree.begin();
        m_states[StateValue::STATE_1] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State1>(this));
        m_states[StateValue::STATE_2] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State2>(this));
        m_states[StateValue::STATE_3] = tree.append_child(m_states[StateValue::STATE_1], std::make_shared<State3>(this));
        m_states[StateValue::UNKNOWN] = tree.end();

        m_next_state = m_states[StateValue::UNKNOWN];

        m_state_manager = std::make_shared<StateManager<ActorBarSuperState_ptr>>(std::move(tree), m_states[StateValue::STATE_1]);
        /* clang-format on */
    }
    ~ActorBar()
    {
    }

    void start()
    {
        m_state_manager->init();
        m_running = true;
        m_thread  = std::thread(&ActorBar::run, this);
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

    SignalIEvent m_signal;

    std::shared_ptr<StateManager<ActorBarSuperState_ptr>>        m_state_manager;
    std::map<StateValue, tree<ActorBarSuperState_ptr>::iterator> m_states;
    tree<ActorBarSuperState_ptr>::iterator                       m_next_state;

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

    std::atomic_bool m_running{false};
    std::thread      m_thread;

    std::shared_ptr<SimplestThreadSafeQueue<IEvent_ptr>> m_queue;
};

/* clang-format off */
int ActorBarSuperState::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int ActorBarSuperState::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; return 0; }
int ActorBarSuperState::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}
int State1::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int State1::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int State1::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventBlue).hash_code())
    {
        auto event_blue = std::dynamic_pointer_cast<Evts::EventBlue>(event);

        if (event_blue)
        {
            LOG_MAIN << "This is the event data: " << event_blue->timeout << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_2];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    return -1;  // Unhandled event
}
int State2::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(1);
    event->data.emplace_back(2);
    event->data.emplace_back(3);
    m_actor->m_signal(event);
    return 0;
}
int State2::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; return 0; }
int State2::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventBlue).hash_code())
    {
        auto event_blue = std::dynamic_pointer_cast<Evts::EventBlue>(event);

        if (event_blue)
        {
            LOG_MAIN << "This is the event data: " << event_blue->timeout << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_3];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    return -1;  // Unhandled event
}
int State3::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(4);
    event->data.emplace_back(5);
    event->data.emplace_back(6);
    m_actor->m_signal(event);
    return 0;
}
int State3::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(7);
    event->data.emplace_back(8);
    event->data.emplace_back(9);
    m_actor->m_signal(event);
    return 0;
}
int State3::process_event(IEvent_ptr event)
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(Evts::EventBlue).hash_code())
    {
        auto event_blue = std::dynamic_pointer_cast<Evts::EventBlue>(event);

        if (event_blue)
        {
            LOG_MAIN << "This is the event data: " << event_blue->timeout << std::endl;
            m_actor->m_next_state = m_actor->m_states[StateValue::STATE_1];
            return 0;
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
    }
    return -1;  // Unhandled event
}
/* clang-format on */
}  // namespace Bar

/* ============================================================================================== */

namespace Demo
{
class App
{
   public:
    App() : m_foo{std::make_shared<Foo::ActorFoo>()}, m_bar{std::make_shared<Bar::ActorBar>()}
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
        m_foo->connect_callbacks(
            boost::bind(&Bar::ActorBar::callback_IEvent, m_bar.get(), boost::placeholders::_1));
        m_bar->connect_callbacks(
            boost::bind(&Foo::ActorFoo::callback_IEvent, m_foo.get(), boost::placeholders::_1));
    }
    ~App()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
        stop();
    }
    void start()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
        m_foo->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
        m_bar->start();
    }
    void stop()
    {
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
        m_foo->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
        m_bar->stop();
    }

   private:
    std::shared_ptr<Foo::ActorFoo> m_foo;
    std::shared_ptr<Bar::ActorBar> m_bar;
};
}  // namespace Demo


/* ============================================================================================== */

int main(int argc, char** argv)
{
    Demo::App application;
    application.start();

    for (uint8_t i = 0; i < 15; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    application.stop();
}