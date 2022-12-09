#include "codetimer.h"
 
Timer::Timer() {
    if (clock_gettime(CLOCK_REALTIME, &_beg_time) == -1) {
        LOG_DEBUG("get time failed.");
    }
    is_stop = false;
}
 
Timer::~Timer() {
    if (!is_stop && !stop()) {
        LOG_DEBUG("get time failed.");
    }
}
 
bool Timer::start() {
    if (clock_gettime(CLOCK_REALTIME, &_beg_time) == -1) {
        LOG_DEBUG("get time failed.");
        return false;
    }
    return true;
}

bool Timer::pause(bool is_reset) {
    if (is_reset) {
        _beg_time = _end_time;
    }
    if (clock_gettime(CLOCK_REALTIME, &_end_time) == -1) {
        LOG_DEBUG("get time failed.");
        return false;
    }

    return true;
}

bool Timer::stop() {
    is_stop = true;
    if (clock_gettime(CLOCK_REALTIME, &_end_time) == -1) {
        LOG_DEBUG("get time failed.");
        return false;
    }
    LOG_DEBUG("execution time: %zu us\n", get_usec_timespan());
    return true;
}
 
size_t Timer::get_sec_timespan() const {
    return _get_timespan(1, 0.000000001);
}
 
size_t Timer::get_msec_timespan() const {
    return _get_timespan(1000, 0.000001);
}
 
size_t Timer::get_usec_timespan() const {
    return _get_timespan(1000000, 0.001);
}
 
size_t Timer::get_nsec_timespan() const {
    return _get_timespan(1000000000, 1);
}
 
size_t Timer::_get_timespan(size_t sec_power, double nsec_power) const {
    double span = (_end_time.tv_sec - _beg_time.tv_sec) * sec_power +
            (_end_time.tv_nsec - _beg_time.tv_nsec) * nsec_power;
    return static_cast<size_t>(span);
}