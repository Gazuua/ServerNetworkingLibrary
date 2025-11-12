#pragma once
#include "macro.h"

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stop_token>
#include <boost/asio.hpp>


NAMESPACE_BEGIN(snl)
class ThreadPool
{
public:
    explicit ThreadPool(size_t concurrency = std::thread::hardware_concurrency())
        : _stop(false)
    {
        for (size_t i = 0; i < concurrency; ++i)
        {
            _threads.emplace_back([this](std::stop_token st)
                {
                    this->process(st);
                });
        }
    }

    ~ThreadPool()
    {
        Shutdown();
    }

    // 작업 추가
    void Enqueue(std::function<void()> job)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _jobs.push(std::move(job));
        }
        _cv.notify_one();
    }

    // 안전하게 풀 종료
    void Shutdown()
    {
        if (_stop)
            return; // 중복 종료 방지

        _stop = true;
        _cv.notify_all();
        for (auto& t : _threads)
        {
            if (t.joinable())
                t.request_stop(); // stop_token 통해 루프 종료 요청
        }
    }

private:
    void process(std::stop_token st)
    {
        while (st.stop_requested() == false)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [this, &st]
                    {
                        return !_jobs.empty() || _stop || st.stop_requested();
                    });

                if (_stop && _jobs.empty())
                    return;

                if (st.stop_requested())
                    return;

                job = std::move(_jobs.front());
                _jobs.pop();
            }

            try
            {
                job();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Job threw exception: " << e.what() << "\n";
            }
        }
    }

private:
    std::vector<std::jthread> _threads;
    std::queue<std::function<void()>> _jobs;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _stop;
};
NAMESPACE_END(snl)
