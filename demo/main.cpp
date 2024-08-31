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
#include "StateManager/StateManager.hpp"

#define LOG_MAIN (LOG("main.cpp", LEVEL_INFO))
#define DELAY 200

using connection = boost::signals2::connection;

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
class Shutdown : public IEvent
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
const std::string& stringify(StateValue state)
{
    static const std::string unknown_state_event = "invalid stringify StateValue";
    static const std::map<int, std::string> stringifier_container = {
        {static_cast<int>(StateValue::UNKNOWN), "StateValue::UNKNOWN"},
        {static_cast<int>(StateValue::ROOT), "StateValue::ROOT"},
        {static_cast<int>(StateValue::STATE_A), "StateValue::STATE_A"},
        {static_cast<int>(StateValue::STATE_B), "StateValue::STATE_B"},
        {static_cast<int>(StateValue::STATE_C), "StateValue::STATE_C"},
        {static_cast<int>(StateValue::STATE_D), "StateValue::STATE_D"},
        {static_cast<int>(StateValue::STATE_E), "StateValue::STATE_E"},
        {static_cast<int>(StateValue::STATE_F), "StateValue::STATE_F"},
        {static_cast<int>(StateValue::STATE_G), "StateValue::STATE_G"}
    };
    auto it = stringifier_container.find(static_cast<int>(state));
    if (it != stringifier_container.end())
    {
        return it->second;
    }
    return unknown_state_event;
}

class ActorFoo;

class ActorFooSuperState
{
   public:
    ActorFooSuperState(ActorFoo* actor, StateValue state = StateValue::ROOT)
        : m_actor{actor}, m_state_enum{state}
    {
    }
    virtual ~ActorFooSuperState(){}
    virtual void on_entry(){}
    virtual void on_exit(){}

    virtual void unhandled_event(IEvent_ptr event)
    {
        (void) event;
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void process_event(IEvent_ptr event)
    {
        (void) event;
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual StateValue type() const { return m_state_enum; }
   protected:
    ActorFoo* m_actor;

   private:
    StateValue m_state_enum;
};

class StateA : public ActorFooSuperState
{
   public:
    virtual ~StateA() = default;
    StateA(ActorFoo* actor, StateValue state = StateValue::STATE_A)
        : ActorFooSuperState(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateC : public StateA
{
   public:
    virtual ~StateC() = default;
    StateC(ActorFoo* actor, StateValue state = StateValue::STATE_C)
        : StateA(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateD : public StateA
{
   public:
    virtual ~StateD() = default;
    StateD(ActorFoo* actor, StateValue state = StateValue::STATE_D)
        : StateA(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateF : public StateD
{
   public:
    virtual ~StateF() = default;
    StateF(ActorFoo* actor, StateValue state = StateValue::STATE_F)
        : StateD(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateG : public StateF
{
   public:
    virtual ~StateG() = default;
    StateG(ActorFoo* actor, StateValue state = StateValue::STATE_G)
        : StateF(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateB : public ActorFooSuperState
{
   public:
    virtual ~StateB() = default;
    StateB(ActorFoo* actor, StateValue state = StateValue::STATE_B)
        : ActorFooSuperState(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateE : public StateB
{
   public:
    virtual ~StateE() = default;
    StateE(ActorFoo* actor, StateValue state = StateValue::STATE_E)
        : StateB(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
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
        m_states[StateValue::STATE_A] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<StateA>(this, StateValue::STATE_A));
        m_states[StateValue::STATE_C] = tree.append_child(m_states[StateValue::STATE_A], std::make_shared<StateC>(this, StateValue::STATE_C));
        m_states[StateValue::STATE_D] = tree.append_child(m_states[StateValue::STATE_A], std::make_shared<StateD>(this, StateValue::STATE_D));
        m_states[StateValue::STATE_F] = tree.append_child(m_states[StateValue::STATE_D], std::make_shared<StateF>(this, StateValue::STATE_F));
        m_states[StateValue::STATE_G] = tree.append_child(m_states[StateValue::STATE_F], std::make_shared<StateG>(this, StateValue::STATE_G));
        m_states[StateValue::STATE_B] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<StateB>(this, StateValue::STATE_B));
        m_states[StateValue::STATE_E] = tree.append_child(m_states[StateValue::STATE_B], std::make_shared<StateE>(this, StateValue::STATE_E));
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
void StateA::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 11;
    m_actor->m_signal(event);
}
void StateA::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_G];
    }
    else
    {

    }
}
void StateA::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateC::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateC::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateC::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
}
void StateD::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateD::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateD::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
}
void StateF::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateF::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateF::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
}
void StateG::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 22;
    m_actor->m_signal(event);
}
void StateG::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateG::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_E];
    }
}
void StateB::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateB::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
}
void StateB::process_event(IEvent_ptr event)
{
    (void) event;
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
}
void StateE::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventBlue> event = std::make_shared<Evts::EventBlue>();
    event->timeout                       = 33;
    m_actor->m_signal(event);
}
void StateE::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void StateE::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_A];
    }
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
const std::string& stringify(StateValue state)
{
    static const std::string unknown_state_event = "invalid stringify StateValue";
    static const std::map<int, std::string> stringifier_container = {
        {static_cast<int>(StateValue::UNKNOWN), "StateValue::UNKNOWN"},
        {static_cast<int>(StateValue::ROOT), "StateValue::ROOT"},
        {static_cast<int>(StateValue::STATE_1), "StateValue::STATE_1"},
        {static_cast<int>(StateValue::STATE_2), "StateValue::STATE_2"},
        {static_cast<int>(StateValue::STATE_3), "StateValue::STATE_3"},
    };
    auto it = stringifier_container.find(static_cast<int>(state));
    if (it != stringifier_container.end())
    {
        return it->second;
    }
    return unknown_state_event;
}

class ActorBar;

class ActorBarSuperState
{
   public:
    ActorBarSuperState(ActorBar* actor, StateValue state = StateValue::ROOT)
        : m_actor{actor}, m_state_enum{state}
    {
    }
    virtual ~ActorBarSuperState(){}
    virtual void on_entry(){}
    virtual void on_exit(){}

    virtual void unhandled_event(IEvent_ptr event)
    {
        (void) event;
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void process_event(IEvent_ptr event)
    {
        (void) event;
        LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual StateValue type() const { return m_state_enum; }
   protected:
    ActorBar* m_actor;

   private:
    StateValue m_state_enum;
};

class State1 : public ActorBarSuperState
{
   public:
    virtual ~State1() = default;
    State1(ActorBar* actor, StateValue state = StateValue::STATE_1)
        : ActorBarSuperState(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class State3 : public State1
{
   public:
    virtual ~State3() = default;
    State3(ActorBar* actor, StateValue state = StateValue::STATE_3)
        : State1(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class State2 : public ActorBarSuperState
{
   public:
    virtual ~State2() = default;
    State2(ActorBar* actor, StateValue state = StateValue::STATE_2)
        : ActorBarSuperState(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
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
        m_states[StateValue::STATE_1] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State1>(this, StateValue::STATE_1));
        m_states[StateValue::STATE_2] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State2>(this, StateValue::STATE_2));
        m_states[StateValue::STATE_3] = tree.append_child(m_states[StateValue::STATE_1], std::make_shared<State3>(this, StateValue::STATE_3));
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
void State1::on_entry() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void State1::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void State1::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_2];
    }
}
void State2::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(1);
    event->data.emplace_back(2);
    event->data.emplace_back(3);
    m_actor->m_signal(event);
}
void State2::on_exit() { LOG_MAIN << __PRETTY_FUNCTION__ << std::endl; }
void State2::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_3];
    }
}
void State3::on_entry()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(4);
    event->data.emplace_back(5);
    event->data.emplace_back(6);
    m_actor->m_signal(event);
}
void State3::on_exit()
{
    LOG_MAIN << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

    std::shared_ptr<Evts::EventGreen> event = std::make_shared<Evts::EventGreen>();
    event->data.emplace_back(7);
    event->data.emplace_back(8);
    event->data.emplace_back(9);
    m_actor->m_signal(event);
}
void State3::process_event(IEvent_ptr event)
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
        }
        else
        {
            LOG_MAIN << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_1];
    }
}
/* clang-format on */
}  // namespace Bar

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