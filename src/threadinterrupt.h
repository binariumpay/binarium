// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_THREADINTERRUPT_H
#define BITCOIN_THREADINTERRUPT_H

#include <atomic>
#include <chrono>
//#include <condition_variable>
#include "/usr/include/boost/thread/condition_variable.hpp"
//#include <mutex>
#include "/usr/include/boost/thread/mutex.hpp"
//#include "/usr/include/c++/7/bits/std_mutex.h"

//#include "/home/rodion/Desktop/SSD/Dash/dash/mingw-std-threads-master/mingw.mutex.h"
//#include "/home/rodion/Desktop/SSD/Dash/dash/mingw-std-threads-master/mingw.thread.h"
//#include "/home/rodion/Desktop/SSD/Dash/dash/mingw-std-threads-master/mingw.condition_variable.h"

/*
    A helper class for interruptible sleeps. Calling operator() will interrupt
    any current sleep, and after that point operator bool() will return true
    until reset.
*/
class CThreadInterrupt
{
public:
    explicit operator bool() const;
    void operator()();
    void reset();
    bool sleep_for(boost::chrono::milliseconds rel_time);
    bool sleep_for(boost::chrono::seconds rel_time);
    bool sleep_for(boost::chrono::minutes rel_time);

private:
    //std::condition_variable cond;
    boost::condition_variable cond;
    //std::mutex mut;
    boost::mutex mut;    
    std::atomic<bool> flag;
};

#endif //BITCOIN_THREADINTERRUPT_H
