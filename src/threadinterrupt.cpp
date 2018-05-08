// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "threadinterrupt.h"

CThreadInterrupt::operator bool() const
{
    return flag.load(std::memory_order_acquire);
}

void CThreadInterrupt::reset()
{
    flag.store(false, std::memory_order_release);
}

void CThreadInterrupt::operator()()
{
    {
        boost::unique_lock<boost::mutex> lock(mut);
        flag.store(true, std::memory_order_release);
    }
    cond.notify_all();
}

bool CThreadInterrupt::sleep_for(boost::chrono::milliseconds rel_time)
{
    boost::unique_lock<boost::mutex> lock(mut);
    return !cond.wait_for(lock, rel_time, [this]() { return flag.load(std::memory_order_acquire); });
}

bool CThreadInterrupt::sleep_for(boost::chrono::seconds rel_time)
{
    return sleep_for(boost::chrono::duration_cast<boost::chrono::milliseconds>(rel_time));
}

bool CThreadInterrupt::sleep_for(boost::chrono::minutes rel_time)
{
    return sleep_for(boost::chrono::duration_cast<boost::chrono::milliseconds>(rel_time));
}
