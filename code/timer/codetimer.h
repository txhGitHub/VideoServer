#include <time.h>
#include <stdio.h>
#include "../log/log.h"
 
class Timer {
public:
     Timer();
     ~Timer();
     bool start();
     bool pause(bool is_reset);
     bool stop();
     size_t get_sec_timespan() const;
     size_t get_msec_timespan() const;
     size_t get_usec_timespan() const;
     size_t get_nsec_timespan() const;
 
private:
     size_t _get_timespan(size_t sec_power, double nsec_power) const;
private:
     struct timespec _beg_time;
     struct timespec _end_time;
     bool is_stop;
};