#include <thread>
#include "async.h"
#include "Bulkmt.h"
#include "BulkImpl.h"

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

            std::shared_ptr<Bulkmt> bulk;
            BulkImpl _bulkImpl;

            std::condition_variable checkCommandLoop;
            std::atomic_bool isDone = false;
            std::thread workerThread;

            Worker(const std::size_t& buffer)
                    : bulk(new Bulkmt(static_cast<int>(buffer)))
                    , _bulkImpl(bulk)
                    , workerThread([this]()
                    {
                        {
                            std::unique_lock<std::mutex> locker(Utils::lockPrint);
                            std::cout << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__ << std::endl;
                        }
                        while (!isDone)
                        {
                            std::unique_lock<std::mutex> locker(_lockCommandLoop);
                            checkCommandLoop.wait(locker, [&]()
                            {
                                return !_commandQueue.empty() || isDone;
                            });

                            if (!_commandQueue.empty())
                            {
                                auto command = std::move(_commandQueue.front());
                                _commandQueue.pop();
                                locker.unlock();

                                bulk->ExecuteAll(command.data, command.size);
                            }
                        }
                        bulk->Finalize();
                        _bulkImpl.Complete();
                    })
            {
            }

            void Add(const char* data, std::size_t size)
            {
                std::unique_lock<std::mutex> locker(_lockCommandLoop);
                if (isDone)
                {
                    return;
                }
                _commandQueue.emplace(data, size);
                checkCommandLoop.notify_one();
            }

            void Complete()
            {
                while (!_commandQueue.empty())
                {
                    checkCommandLoop.notify_one();
                }

                isDone = true;
                checkCommandLoop.notify_one();
            }

        private:
            std::mutex _lockCommandLoop;
            std::queue<Command> _commandQueue;
    };


    handle_t connect(std::size_t bulk)
    {
        _contextCache.emplace_back(new Worker(bulk));
        auto& ctx = _contextCache[_contextCache.size() - 1];
        return ctx.get();
    }

    void receive(handle_t handle, const char* data, std::size_t size)
    {
        if (handle == nullptr)
        {
            return;
        }

        static_cast<Worker*>(handle)->Add(data, size);
    }

    void disconnect(handle_t handle)
    {
        auto* worker = static_cast<Worker*>(handle);
        worker->Complete();
        worker->workerThread.join();

        _contextCache.erase(std::find_if(_contextCache.begin(), _contextCache.end(), [handle](auto&& item)
        {
            return item.get() == handle;
        }));
    }

}
