#include <map>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <atomic>

#include <boost/signals2.hpp>

#include "ThreadSafeQueue.h"
#include "StateManager.h"

using connection   = boost::signals2::connection;
using IEventSignal = boost::signals2::signal<void(IEvent_ptr)>;

namespace A
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

class ActorA;

class ActorASuperState
{
   public:
    ActorASuperState(ActorA* actor, StateValue state = StateValue::ROOT)
        : m_actor{actor}, m_state_enum{state}
    {
    }
    virtual ~ActorASuperState(){}
    virtual void on_entry(){}
    virtual void on_exit(){}

    virtual void unhandled_event(IEvent_ptr event)
    {
        (void) event;
        std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void process_event(IEvent_ptr event)
    {
        (void) event;
        std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual StateValue type() const { return m_state_enum; }
   protected:
    ActorA* m_actor;

   private:
    StateValue m_state_enum;
};

class StateA : public ActorASuperState
{
   public:
    virtual ~StateA() = default;
    StateA(ActorA* actor, StateValue state = StateValue::STATE_A)
        : ActorASuperState(actor, state)
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
    StateC(ActorA* actor, StateValue state = StateValue::STATE_C)
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
    StateD(ActorA* actor, StateValue state = StateValue::STATE_D)
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
    StateF(ActorA* actor, StateValue state = StateValue::STATE_F)
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
    StateG(ActorA* actor, StateValue state = StateValue::STATE_G)
        : StateF(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class StateB : public ActorASuperState
{
   public:
    virtual ~StateB() = default;
    StateB(ActorA* actor, StateValue state = StateValue::STATE_B)
        : ActorASuperState(actor, state)
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
    StateE(ActorA* actor, StateValue state = StateValue::STATE_E)
        : StateB(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

/* clang-format on */
class ActorA
{
   public:
    using ActorASuperState_ptr = std::shared_ptr<ActorASuperState>;
    ActorA() : m_queue{std::make_shared<SimplestThreadSafeQueue<IEvent_ptr>>()}
    {
        /* clang-format off */
        tree<ActorASuperState_ptr> tree;
        tree.set_head(std::make_shared<ActorASuperState>(this));
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

        m_state_manager = std::make_shared<StateManager<ActorASuperState_ptr>>(std::move(tree), m_states[StateValue::STATE_A]);
        /* clang-format on */
    }
    ~ActorA()
    {
    }

    void start()
    {
        m_state_manager->init();
        m_running = true;
        m_thread  = std::thread(&ActorA::run, this);
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
    connection connect_callback_for_signal_emited_from_A(T&& handler)
    {
        return m_signal.connect(handler);
    }

    IEventSignal m_signal;

    std::shared_ptr<StateManager<ActorASuperState_ptr>>        m_state_manager;
    std::map<StateValue, tree<ActorASuperState_ptr>::iterator> m_states;
    tree<ActorASuperState_ptr>::iterator                       m_next_state;

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
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleA> event = std::make_shared<EventExampleA>();
    event->timeout                       = 11;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void StateA::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleB).hash_code())
    {
        auto event_example_b = std::dynamic_pointer_cast<EventExampleB>(event);

        if (event_example_b)
        {
            std::cout << "This is the event data: " << std::endl;
            for (auto k : event_example_b->data)
            {
                std::cout << k << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_G];
    }
}
void StateA::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateC::on_entry() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateC::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateC::process_event(IEvent_ptr event)
{
    (void) event;
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
}
void StateD::on_entry() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateD::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateD::process_event(IEvent_ptr event)
{
    (void) event;
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
}
void StateF::on_entry() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateF::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateF::process_event(IEvent_ptr event)
{
    (void) event;
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
}
void StateG::on_entry()
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleA> event = std::make_shared<EventExampleA>();
    event->timeout                       = 22;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void StateG::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateG::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleB).hash_code())
    {
        auto event_example_b = std::dynamic_pointer_cast<EventExampleB>(event);

        if (event_example_b)
        {
            std::cout << "This is the event data: " << std::endl;
            for (auto k : event_example_b->data)
            {
                std::cout << k << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_E];
    }
}
void StateB::on_entry() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateB::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateB::process_event(IEvent_ptr event)
{
    (void) event;
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
}
void StateE::on_entry()
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleA> event = std::make_shared<EventExampleA>();
    event->timeout                       = 33;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void StateE::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void StateE::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleB).hash_code())
    {
        auto event_example_b = std::dynamic_pointer_cast<EventExampleB>(event);

        if (event_example_b)
        {
            std::cout << "This is the event data: " << std::endl;
            for (auto k : event_example_b->data)
            {
                std::cout << k << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_A];
    }
}

/* clang-format on */
}  // namespace A

/* ============================================================================================== */

namespace B
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

class ActorB;

class ActorBSuperState
{
   public:
    ActorBSuperState(ActorB* actor, StateValue state = StateValue::ROOT)
        : m_actor{actor}, m_state_enum{state}
    {
    }
    virtual ~ActorBSuperState(){}
    virtual void on_entry(){}
    virtual void on_exit(){}

    virtual void unhandled_event(IEvent_ptr event)
    {
        (void) event;
        std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void process_event(IEvent_ptr event)
    {
        (void) event;
        std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual StateValue type() const { return m_state_enum; }
   protected:
    ActorB* m_actor;

   private:
    StateValue m_state_enum;
};

class State1 : public ActorBSuperState
{
   public:
    virtual ~State1() = default;
    State1(ActorB* actor, StateValue state = StateValue::STATE_1)
        : ActorBSuperState(actor, state)
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
    State3(ActorB* actor, StateValue state = StateValue::STATE_3)
        : State1(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};

class State2 : public ActorBSuperState
{
   public:
    virtual ~State2() = default;
    State2(ActorB* actor, StateValue state = StateValue::STATE_2)
        : ActorBSuperState(actor, state)
    {
    }
    virtual void on_entry() override;
    virtual void on_exit() override;
    virtual void process_event(IEvent_ptr event) override;
};


/* clang-format on */
class ActorB
{
   public:
    using ActorBSuperState_ptr = std::shared_ptr<ActorBSuperState>;
    ActorB() : m_queue{std::make_shared<SimplestThreadSafeQueue<IEvent_ptr>>()}
    {
        /* clang-format off */
        tree<ActorBSuperState_ptr> tree;
        tree.set_head(std::make_shared<ActorBSuperState>(this));
        m_states[StateValue::ROOT] = tree.begin();
        m_states[StateValue::STATE_1] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State1>(this, StateValue::STATE_1));
        m_states[StateValue::STATE_2] = tree.append_child(m_states[StateValue::ROOT], std::make_shared<State2>(this, StateValue::STATE_2));
        m_states[StateValue::STATE_3] = tree.append_child(m_states[StateValue::STATE_1], std::make_shared<State3>(this, StateValue::STATE_3));
        m_states[StateValue::UNKNOWN] = tree.end();

        m_next_state = m_states[StateValue::UNKNOWN];

        m_state_manager = std::make_shared<StateManager<ActorBSuperState_ptr>>(std::move(tree), m_states[StateValue::STATE_1]);
        /* clang-format on */
    }
    ~ActorB()
    {
    }

    void start()
    {
        m_state_manager->init();
        m_running = true;
        m_thread  = std::thread(&ActorB::run, this);
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
    connection connect_callback_for_signal_emited_from_B(T&& handler)
    {
        return m_signal.connect(handler);
    }

    IEventSignal m_signal;

    std::shared_ptr<StateManager<ActorBSuperState_ptr>>        m_state_manager;
    std::map<StateValue, tree<ActorBSuperState_ptr>::iterator> m_states;
    tree<ActorBSuperState_ptr>::iterator                       m_next_state;

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
void State1::on_entry() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void State1::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void State1::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleA).hash_code())
    {
        auto event_example_a = std::dynamic_pointer_cast<EventExampleA>(event);

        if (event_example_a)
        {
            std::cout << "This is the event data: " << event_example_a->timeout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_2];
    }
}
void State2::on_entry()
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleB> event = std::make_shared<EventExampleB>();
    event->data.emplace_back(1);
    event->data.emplace_back(2);
    event->data.emplace_back(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void State2::on_exit() { std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl; }
void State2::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleA).hash_code())
    {
        auto event_example_a = std::dynamic_pointer_cast<EventExampleA>(event);

        if (event_example_a)
        {
            std::cout << "This is the event data: " << event_example_a->timeout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_3];
    }
}
void State3::on_entry()
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleB> event = std::make_shared<EventExampleB>();
    event->data.emplace_back(4);
    event->data.emplace_back(5);
    event->data.emplace_back(6);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void State3::on_exit()
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;

    std::shared_ptr<EventExampleB> event = std::make_shared<EventExampleB>();
    event->data.emplace_back(7);
    event->data.emplace_back(8);
    event->data.emplace_back(9);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_actor->m_signal(event);
}
void State3::process_event(IEvent_ptr event)
{
    std::cout << "Method called: " << __PRETTY_FUNCTION__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::size_t event_type = event->getTypeHash();

    if (event_type == typeid(EventExampleA).hash_code())
    {
        auto event_example_a = std::dynamic_pointer_cast<EventExampleA>(event);

        if (event_example_a)
        {
            std::cout << "This is the event data: " << event_example_a->timeout << std::endl;
        }
        else
        {
            std::cout << "Downcasting failed." << std::endl;
        }
        m_actor->m_next_state = m_actor->m_states[StateValue::STATE_1];
    }
}
/* clang-format on */
}  // namespace B

int main(void)
{
    std::shared_ptr<A::ActorA> actA = std::make_shared<A::ActorA>();
    std::shared_ptr<B::ActorB> actB = std::make_shared<B::ActorB>();

    actA->connect_callback_for_signal_emited_from_A(
        boost::bind(&B::ActorB::callback_IEvent, actB.get(), _1));
    actB->connect_callback_for_signal_emited_from_B(
        boost::bind(&A::ActorA::callback_IEvent, actA.get(), _1));

    actA->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    actB->start();

    for (uint8_t i = 0; i < 50; i++)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    actA->stop();
    actB->stop();
}