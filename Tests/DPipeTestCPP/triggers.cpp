#include "triggers.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace crdk::triggers;

typedef std::chrono::high_resolution_clock Time;

bool Trigger::IsComplete() const {
    return _status >= 1;
}

bool crdk::triggers::Trigger::IsSucces() const
{
    return _status == TRIGGER_STATUS::COMPLETE;
}

void BoolTrigger::SetComplete() {
    _status = 1;
}

void crdk::triggers::wait(Trigger& trigger, unsigned int timeout) {

    auto timeBegin = Time::now();

    while (!trigger.IsComplete()) {

        this_thread::sleep_for(1ms);

        if (timeout > 0) {
            chrono::duration<float> duraction = Time::now() - timeBegin;
            auto waitingTime = chrono::duration_cast<chrono::milliseconds>(duraction).count();

            if (waitingTime > timeout)
                break;
        }
    }
}

bool crdk::triggers::EQUAL_COMPARATOR(int current, int target) {
    return current == target;
}

bool crdk::triggers::MORE_COMPARATOR(int current, int target) {
    return current > target;
}

bool crdk::triggers::MORE_OR_EQUAL_COMPARATOR(int current, int target) {
    return current >= target;
}

bool crdk::triggers::LESS_COMPARATOR(int current, int target) {
    return current < target;
}

bool crdk::triggers::LESS_OR_EQUAL_COMPARATOR(int current, int target) {
    return current <= target;
}

crdk::triggers::IntTrigger::IntTrigger(int targetNumber, std::function<bool(int, int)> comparator) {
    _target = targetNumber;
    _comparator = comparator;
}

crdk::triggers::IntTrigger::IntTrigger(int startNumber, int targetNumber, std::function<bool(int, int)> comparator) {
    _current = startNumber; 
    _target = targetNumber;
    _comparator = comparator;
}

void crdk::triggers::IntTrigger::Increase(int number) {
    _current += number;

    if (_comparator(_current, _target))
        _status = TRIGGER_STATUS::COMPLETE;
}

void crdk::triggers::IntTrigger::Decrease(int number) {
    _current -= number;

    if (_comparator(_current, _target))
        _status = TRIGGER_STATUS::COMPLETE;
}
