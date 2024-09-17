#ifndef __ISTATE_H_
#define __ISTATE_H_

#include "IEvent/IEvent.hpp"

template <class Actor>
class IState
{
   public:
    IState(Actor* actor) : m_actor{actor}
    {
    }
    virtual ~IState()
    {
    }

    virtual int on_entry()
    {
        return 0;  // no error
    }

    virtual int on_exit()
    {
        return 0;  // no error
    }

    /**
     * Must return 0 when the event has been handled in that state
     */
    virtual int process_event(IEvent_ptr event)
    {
        (void) event;
        return -1;  // Unhandled event
    }

   protected:
    Actor* m_actor;
};

#endif