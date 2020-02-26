#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <typeindex>
#include <queue>
#include <zconf.h>
#include "Sequence.h"
#include "InfinitSequence.h"
#include "events/EventDispatcher.h"


class Sequence;

struct Metrics
{
    int lineCount = 0u;
    int commandCount = 0u;
    int blockCount = 0u;
};

class Bulkmlt
{
    private:
        std::map<std::type_index, std::shared_ptr<IInterpreterState>> _typeToInterpreter;

    public:

        Metrics mainMetrics;

        EventDispatcher<std::shared_ptr<Group>> eventSequenceComplete;
        EventDispatcher<time_t> eventFirstCommand;
        EventDispatcher<std::string> eventCommandPush;

        Bulkmlt(int commandBufCount)
                : commandBufCount(commandBufCount)
        {
            SetState<Sequence>();
        };

        template<typename T>
        void SetState()
        {
            if (_currentState)
            {
                _currentState->Finalize();
            }

            auto typeIndex = std::type_index(typeid(std::remove_reference_t<T>));
            auto it = _typeToInterpreter.find(typeIndex);
            if (it == _typeToInterpreter.end())
            {
                std::tie(it, std::ignore) = _typeToInterpreter.emplace(std::piecewise_construct, std::make_tuple(typeIndex), std::make_tuple(new T{
                        *this}));
            }
            _currentState = it->second;

            _currentState->Initialize();
        }

        std::shared_ptr<IInterpreterState> GetState()
        {
            return _currentState;
        }

        void Execute(const std::string& command)
        {
            _currentState->Exec(command);
            mainMetrics.lineCount++;
        }

        void ExecuteAll(const char* data, size_t size)
        {
            std::stringstream buffer;
            buffer << data;

            std::string command;
            while (true)
            {
                std::getline(buffer, command);
                if (std::cin.eof())
                {
                    _currentState->Finalize();
                    break;
                }
//                std::cout << "Input is: " << command << " Processing... " << std::endl;
                Execute(command);
            }
        }

       /* void Run()
        {
//            std::cout << __PRETTY_FUNCTION__ << std::endl;
            while (true)
            {
                std::cout << "Waiting for input:" << std::endl;
                std::string command;
                std::getline(std::cin, command);
                if (std::cin.eof())
                {
                    _currentState->Finalize();
                    break;
                }
                std::cout << "Input is: " << command << " Processing... " << std::endl;
                _currentState->Exec(command);
                mainMetrics.lineCount++;
            }
            std::cout << "Input complete aborting" << std::endl;
        };
*/

        int commandBufCount;
    private:
        std::shared_ptr<IInterpreterState> _currentState;
};
