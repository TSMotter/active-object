#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <memory>

#include "Logger/Logger.hpp"
#include "ThreadSafeQueue/ThreadSafeQueue.hpp"

#define LOG_MAIN(lvl) (LOG("main.cpp", lvl))

void sigint_handler(int sig)
{
    printf("[sigint_handler] Will exit cleanly...\n");
    exit(0);
}

int main(int argc, char **argv)
{
    /* Binds the SIGINT signal to my custom handler */
    signal(SIGINT, sigint_handler);

    LOG_MAIN(LEVEL_ERROR) << "Hello World" << std::endl;
    LOG_MAIN(LEVEL_DEBUG) << "Good bye World" << std::endl;

    {
        std::shared_ptr<tsq::SimplestThreadSafeQueue<int>> m_queue;
        m_queue = std::make_shared<tsq::SimplestThreadSafeQueue<int>>();
        m_queue->put(2);
        m_queue->wait_and_pop();
    }

    return 0;
}