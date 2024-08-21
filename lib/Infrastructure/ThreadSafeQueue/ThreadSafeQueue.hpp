#ifndef __THREADSAFEQUEUE__
#define __THREADSAFEQUEUE__

#include <condition_variable>
#include <mutex>
#include <deque>

#include <chrono>
#include <memory>
#include <thread>

#include "Logger/Logger.hpp"

#define LOG_TSQ(lvl) (LOG("ThreadSafeQueue.hpp", lvl))

namespace tsq
{
template <typename T>
class IThreadSafeQueue
{
   public:
    virtual void put(T element)                                             = 0;
    virtual void put_prioritized(T element)                                 = 0;
    virtual T    wait_and_pop()                                             = 0;
    virtual T    wait_and_pop_for(const std::chrono::milliseconds &timeout) = 0;
    virtual bool empty()                                                    = 0;
    virtual void reset()                                                    = 0;
    virtual void clear()                                                    = 0;

   private:
};

template <typename T>
class SimplestThreadSafeQueue : public IThreadSafeQueue<T>
{
   public:
    SimplestThreadSafeQueue()
    {
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void put(T element) override
    {
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
            m_queue.push_back(element);
        }
        m_cv.notify_all();
    }
    virtual void put_prioritized(T element) override
    {
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
            m_queue.push_front(element);
        }
        m_cv.notify_all();
    }
    // Wait without a timeout
    virtual T wait_and_pop() override
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
        m_cv.wait(lock,
                  [&]()
                  {
                      LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << " - Wait" << std::endl;
                      return !m_queue.empty();
                  });
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << " - Finished waiting" << std::endl;
        T result = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        return result;
    }

    // Wait with a timeout
    virtual T wait_and_pop_for(const std::chrono::milliseconds &timeout) override
    {
        T                            result;
        std::unique_lock<std::mutex> lock(m_mutex);
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
        /*  The return of wait_for is false if it returns and the predicate is still false */
        if (m_cv.wait_for(lock, timeout,
                          [&]()
                          {
                              LOG_TSQ(LEVEL_DEBUG)
                                  << __PRETTY_FUNCTION__ << " - Waiting" << std::endl;
                              return !m_queue.empty();
                          }))
        {
            LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << " - Finished waiting" << std::endl;
            result = m_queue.front();
            m_queue.pop_front();
            lock.unlock();
        }
        return result;
    }
    virtual bool empty() override
    {
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
        bool result;
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            result = m_queue.empty();
        }
        return result;
    }
    virtual void reset() override
    {
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
        m_queue = std::deque<T>{};
    }
    virtual void clear() override
    {
        LOG_TSQ(LEVEL_DEBUG) << __PRETTY_FUNCTION__ << std::endl;
        m_queue.clear();
    }

   private:
    std::deque<T>           m_queue{};
    std::condition_variable m_cv;
    std::mutex              m_mutex;
};

}  // namespace tsq

#endif