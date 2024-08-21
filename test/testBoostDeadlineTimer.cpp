#include <gtest/gtest.h>

#include "BoostDeadlineTimer.hpp"

// Fixture definition
class BoostDeadlineTimerFixture : public ::testing::Test
{
   protected:
    BoostDeadlineTimerFixture()
        : m_timer{m_default_period, boost::bind(&BoostDeadlineTimerFixture::callback, this), true},
          m_callback_counter(0)
    {
        // You can do set-up work for each test here.
    }

    ~BoostDeadlineTimerFixture()
    {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    void callback()
    {
        // std::cout << "BoostDeadlineTimerFixture::callback()" << std::endl;
        m_callback_counter++;
    }

    static const long m_safe_margin    = 20;   // milliseconds
    static const long m_default_period = 200;  // milliseconds
    DeadlineTimer     m_timer;
    int               m_callback_counter;
};

TEST_F(BoostDeadlineTimerFixture, TestStartDefault)
{
    m_timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200 + m_safe_margin));
    ASSERT_EQ(1, m_callback_counter);

    std::this_thread::sleep_for(std::chrono::milliseconds(400 + m_safe_margin));
    ASSERT_EQ(3, m_callback_counter);
}

TEST_F(BoostDeadlineTimerFixture, TestStartChangingPeriod)
{
    m_timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(300 + m_safe_margin));
    ASSERT_EQ(1, m_callback_counter);

    m_timer.start(50);
    std::this_thread::sleep_for(std::chrono::milliseconds(200 + m_safe_margin));
    ASSERT_EQ(5, m_callback_counter);
}

TEST_F(BoostDeadlineTimerFixture, TestStartChangingCyclic)
{
    m_timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(300 + m_safe_margin));
    ASSERT_EQ(1, m_callback_counter);

    m_timer.start(50, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(500 + m_safe_margin));
    ASSERT_EQ(2, m_callback_counter);
}

TEST_F(BoostDeadlineTimerFixture, TestStop)
{
    m_timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200 + m_safe_margin));
    ASSERT_EQ(1, m_callback_counter);

    m_timer.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    ASSERT_EQ(1, m_callback_counter);
}

TEST_F(BoostDeadlineTimerFixture, TestStatus)
{
    ASSERT_EQ(DeadlineTimer::Status::stopped, m_timer.status());
    m_timer.start(100, false);
    ASSERT_EQ(DeadlineTimer::Status::running, m_timer.status());
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + m_safe_margin));
    ASSERT_EQ(DeadlineTimer::Status::stopped, m_timer.status());
}