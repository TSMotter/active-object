#ifndef __STATEMANAGER_H_
#define __STATEMANAGER_H_

#include "tree/tree.h"
#include "IEvent/IEvent.hpp"

template <typename T>
class StateManager
{
   public:
    using t_iterator = typename tree<T>::iterator;

    StateManager(tree<T>&& state_tree, t_iterator current_state)
        : m_tree(std::move(state_tree)), m_current_state(current_state)
    {
    }

    void transitionTo(t_iterator target_state)
    {
        // Special case: self-transition
        if (target_state == m_current_state)
        {
            m_current_state.node->data->on_exit();
            m_current_state.node->data->on_entry();
            return;
        }

        std::vector<t_iterator> ancestorsA;
        std::vector<t_iterator> ancestorsB;

        for (auto it = m_current_state; it != m_tree.begin(); it = m_tree.parent(it))
        {
            ancestorsA.push_back(it);
        }

        for (auto it = target_state; it != m_tree.begin(); it = m_tree.parent(it))
        {
            ancestorsB.insert(ancestorsB.begin(), it);
        }

        t_iterator                                 commonAncestor = m_tree.begin();
        typename std::vector<t_iterator>::iterator it_commonAncestor;
        for (auto& it : ancestorsA)
        {
            it_commonAncestor = std::find(ancestorsB.begin(), ancestorsB.end(), it);
            if (it_commonAncestor != ancestorsB.end())
            {
                commonAncestor = it;
                break;
            }
            else
            {
                it.node->data->on_exit();
            }
        }

        /* Did not find common ancestor */
        if (it_commonAncestor == ancestorsB.end())
        {
            it_commonAncestor = ancestorsB.begin();
        }
        else
        {
            it_commonAncestor++;
        }

        for (auto it = it_commonAncestor; it != ancestorsB.end(); ++it)
        {
            (*it).node->data->on_entry();
        }
        m_current_state = target_state;
    }

    void init()
    {
        // std::cout << "This is m_current_state: " << typeid(m_current_state).name() << std::endl;
        m_current_state.node->data->on_entry();
    }

    void processEvent(std::shared_ptr<IEvent> event)
    {
        if (m_current_state.node->data->process_event(event) != 0)
        {
            t_iterator parent_state = m_tree.parent(m_current_state);
            while ((parent_state.node->data->process_event(event) != 0)
                   && (parent_state != m_tree.begin()))
            {
                parent_state = m_tree.parent(parent_state);
            }
        }
    }

    void currentState(t_iterator current_state)
    {
        m_current_state = current_state;
    }

    t_iterator currentState()
    {
        return m_current_state;
    }

   private:
    tree<T>    m_tree;
    t_iterator m_current_state;
};

#endif