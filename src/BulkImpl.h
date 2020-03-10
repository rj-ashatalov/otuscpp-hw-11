#pragma once

#include "Bulkmt.h"
#include <iostream>
#include <regex>
#include "log/ConsoleLogger.h"
#include "log/FileLogger.h"
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <atomic>

class BulkImpl
{
    private:
        std::shared_ptr<Bulkmt> _bulk;

        std::mutex lockLoggerQueue;

        std::mutex lockFileQueue;

        std::condition_variable threadLogCheck;
        std::condition_variable threadFileCheck;

        std::atomic_bool isDone = false;

        std::queue<std::future<void>> loggerQueue;
        std::queue<std::future<int>> fileQueue;

        Metrics logMetrics;
        FileLogger fileLogger;
        Metrics fileMetricsOne;
        Metrics fileMetricsTwo;
        ConsoleLogger consoleLogger;

        std::thread log;
        std::thread file1;
        std::thread file2;

        void FileLogWorker(Metrics& fileMetrics)
        {
            {
                std::unique_lock<std::mutex> locker(Utils::lockPrint);
                std::cout << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__ << std::endl;
            }
            while (!isDone)
            {
                std::unique_lock<std::mutex> locker(lockFileQueue);
                threadFileCheck.wait(locker, [&]()
                {
                    return !fileQueue.empty() || isDone;
                });

                if (!fileQueue.empty())
                {
                    auto file = std::move(fileQueue.front());
                    fileQueue.pop();
                    locker.unlock();

                    fileMetrics.blockCount++;
                    fileMetrics.commandCount += file.get();
                }
            }
        }

    public:
        BulkImpl(std::shared_ptr<Bulkmt>& bulk)
                : _bulk(bulk)
                , logMetrics{}
                , fileLogger{}
                , fileMetricsOne{}
                , fileMetricsTwo{}
                , consoleLogger{}
                , log([this]()
                {
                    {
                        std::unique_lock<std::mutex> locker(Utils::lockPrint);
                        std::cout << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__ << std::endl;
                    }
                    while (!isDone)
                    {
                        std::unique_lock<std::mutex> locker(lockLoggerQueue);
                        threadLogCheck.wait(locker, [&]()
                        {
                            return !loggerQueue.empty() || isDone;
                        });

                        if (!loggerQueue.empty())
                        {
                            auto content = std::move(loggerQueue.front());
                            loggerQueue.pop();
                            locker.unlock();

                            content.get();
                        }
                    }
                })
                , file1([this](Metrics& fileMetrics)
                {
                    FileLogWorker(fileMetrics);
                }, std::ref(fileMetricsOne))
                , file2([this](Metrics& fileMetrics)
                {
                    FileLogWorker(fileMetrics);
                }, std::ref(fileMetricsTwo))
        {
            _bulk->eventSequenceComplete.Subscribe([&](auto&& group)
            {
                _bulk->mainMetrics.blockCount++;
                _bulk->mainMetrics.commandCount += group->Size();
            });

            _bulk->eventSequenceComplete.Subscribe([&](auto&& group)
            {
                {
                    std::lock_guard<std::mutex> lock(lockLoggerQueue);
                    loggerQueue.emplace(std::async(std::launch::deferred, [&](const auto& group)
                    {
                        logMetrics.blockCount++;
                        logMetrics.commandCount += group->Size();
                        consoleLogger.Log("_bulk: " + static_cast<std::string>(*group));
                    }, group));
                }
                threadLogCheck.notify_one();
            });

            _bulk->eventFirstCommand.Subscribe([&](auto&& timestamp)
            {
                std::lock_guard<std::mutex> lock(lockFileQueue);

                std::stringstream currentTime;
                currentTime << timestamp << "_" << std::to_string(_bulk->mainMetrics.lineCount);
                fileLogger.PrepareFilename("_bulk" + currentTime.str());
            });

            _bulk->eventSequenceComplete.Subscribe([&](auto&& group)
            {
                {
                    std::lock_guard<std::mutex> lock(lockFileQueue);
                    fileQueue.emplace(std::async(std::launch::deferred, [&](const auto& filename, const auto& content) -> int
                    {
                        fileLogger.Log(filename, content);
                        return content->Size();
                    }, fileLogger.GetFileName(), group));
                }
                threadFileCheck.notify_one();
            });
        }

        void Complete()
        {
            while (!loggerQueue.empty() || !fileQueue.empty())
            {
                threadLogCheck.notify_one();
                threadFileCheck.notify_one();
            }

            isDone = true;
            threadLogCheck.notify_all();
            threadFileCheck.notify_all();

            log.join();
            file1.join();
            file2.join();

            {
                std::unique_lock<std::mutex> locker(Utils::lockPrint);
                std::cout << "=== " << std::this_thread::get_id() << " ===" << std::endl;
                std::cout << "common поток - " << _bulk->mainMetrics.lineCount << " строк, "
                          << _bulk->mainMetrics.commandCount << " команд, "
                          << _bulk->mainMetrics.blockCount << " блок" << std::endl;

                std::cout << "log поток - " << logMetrics.blockCount << " блок, "
                          << logMetrics.commandCount << " команд, " << std::endl;

                std::cout << "file1 поток - " << fileMetricsOne.blockCount << " блок, "
                          << fileMetricsOne.commandCount << " команд, " << std::endl;

                std::cout << "file2 поток - " << fileMetricsTwo.blockCount << " блок, "
                          << fileMetricsTwo.commandCount << " команд, " << std::endl;
                std::cout << "=== " << std::this_thread::get_id() << " ===" << std::endl;
            }
        }
};