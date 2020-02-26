#include <thread>
#include "async.h"
#include "Bulkmlt.h"

namespace async
{
    struct Worker: public std::thread
    {
            struct Command
            {
                Command(const char* data, std::size_t size)
                        : data(data)
                        , size(size)
                {
                }

                Command(Command&& other)
                        : data(std::move(other.data))
                        , size(std::move(other.size))
                {
                }

                const char* data;
                std::size_t size;
            };

            std::shared_ptr<Bulkmlt> bulk;
            std::condition_variable checkCommandLoop;

            Worker(const std::size_t& buffer)
                    : bulk(new Bulkmlt(static_cast<int>(buffer)))
                    , std::thread([this]()
                    {
                        {
                            std::unique_lock<std::mutex> locker(lockPrint);
                            std::cout << __PRETTY_FUNCTION__ << std::endl;
                        }
                        while (!_isDone)
                        {
                            std::unique_lock<std::mutex> locker(_lockCommandLoop);
                            checkCommandLoop.wait(locker, [&]()
                            {
                                return !_commandQueue.empty() || _isDone;
                            });

                            if (!_commandQueue.empty())
                            {
                                auto command = std::move(_commandQueue.front());
                                _commandQueue.pop();
                                locker.unlock();

                                bulk->ExecuteAll(command.data, command.size);
                            }
                        }
                    })
            {
            }

            void Add(const char* data, std::size_t size)
            {
                std::unique_lock<std::mutex> locker(_lockCommandLoop);
                _commandQueue.emplace(data, size);
                checkCommandLoop.notify_one();
            }

        private:
            std::atomic_bool _isDone;
            std::mutex _lockCommandLoop;

            std::queue<Command> _commandQueue;
    };


    handle_t connect(std::size_t bulk)
    {
        auto ctx = _contextCatch.emplace_back(new Worker(bulk));
        return ctx->Add;
    }

    void receive(handle_t handle, const char* data, std::size_t size)
    {
//        handle->AddCommand(data, size);
    }

    void disconnect(handle_t handle)
    {
        _contextCatch.erase(std::find(_contextCatch.begin(), _contextCatch.end(), handle));
    }

}
