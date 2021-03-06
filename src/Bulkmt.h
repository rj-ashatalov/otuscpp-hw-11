#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <typeindex>
#include <queue>
#include <mutex>
#include <zconf.h>
#include "Sequence.h"
#include "InfinitSequence.h"
#include "events/EventDispatcher.h"
#include "utils/utils.h"

class Sequence;

struct Metrics
{
    int lineCount = 0u;
    int commandCount = 0u;
    int blockCount = 0u;
};

class Bulkmt
{
    private:
        std::map<std::type_index, std::shared_ptr<IInterpreterState>> _typeToInterpreter;

    public:

        Metrics mainMetrics;

        EventDispatcher<std::shared_ptr<Group>> eventSequenceComplete;
        EventDispatcher<time_t> eventFirstCommand;
        EventDispatcher<std::string> eventCommandPush;

        Bulkmt(int commandBufCount)
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

        void ExecuteAll(const std::string& data, size_t size)
        {
            if (size == 1 && *data.data() == '\0')
            {
                SetState<Sequence>();
                return;
            }

            for (auto ch : data)
            {
                if (ch != '\n')
                {
                    _line << ch;
                }
                else
                {
                    Execute(_line.str());
                    _line.str("");
                }
            }
        }

        void Finalize()
        {
            auto&& command = _line.str();
            if (command.size() > 0)
            {
                Execute(command);
            }
            _currentState->Finalize();
        }

        int commandBufCount;
    private:
        std::shared_ptr<IInterpreterState> _currentState;

        std::stringstream _line;
};
