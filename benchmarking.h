#pragma once
/* Simple benchmark harness for different implementations.

  To compile:
    # without CPU specific code paths:
    g++ -Ofast file.cpp -o bsf

    # with CPU specific SSE4.2 code paths:
    g++ -Ofast file.cpp -msse4.2 -o bsf2

    Before running: Make sure that your CPUs are not in scaling mode by running the below:
        sudo cpupower frequency-set --min 2100M --max 2100M
        sudo cpupower frequency-set --governor performance

    To find out the speed of your processor and how many cores you have:
        cat /proc/cpuinfo | egrep "(GHz|processor|MHz)" | tr "\n" "\t" | sed 's/processor/\nprocessor/g' | awk 1

    NOTE: Some interesting reading about clock times:
        https://software.intel.com/en-us/forums/software-tuning-performance-optimization-platform-monitoring/topic/721139
*/
#include <sys/time.h> // timespec
#include <cstdarg>    // va_list
#include <sched.h>    // sched_getcpu
#include <unistd.h>   // usleep
#include <cstdint>    // uint64_t
#include <string>
#include <iostream>
#include <iomanip>
#include <csignal>    // sigaction
#include <cstring>    // memset

namespace benchmark {

// Useful shortcuts
constexpr uint64_t NSEC_PER_SEC{1000000000UL};
constexpr uint64_t USEC_PER_SEC{NSEC_PER_SEC/1000};
constexpr uint64_t MSEC_PER_SEC{USEC_PER_SEC/1000};

// start - calibration variables
// set automatically at beginning of program by using SIGNAL or LOOP calibration
enum CalibrationType: uint8_t { SIGNAL, LOOP, TotalTypes };
static double g_ticks_per_nanosec[CalibrationType::TotalTypes]={}; //0-SIGALRM calibrated, 1-LOOP calibrated
constexpr uint64_t CALIBRATE_LOOPS{100000000UL};
static struct timespec timer_prev{};
static uint64_t timer_rdtsc_prev{};
constexpr uint8_t TIMER_INTERVAL_IN_US{100}; // in usec
constexpr uint16_t TIMER_LOOPS{USEC_PER_SEC/TIMER_INTERVAL_IN_US};
bool waiting_for_signal{true};
// end calibration variables

// How many loops to run each test through?
constexpr uint64_t ITERATIONS{1000000ULL};

// ticks are set either by loop or signal initialization above
static inline double get_nanos_from_ticks(uint64_t ticks)
{
    return ticks/g_ticks_per_nanosec[0];
}

// keeps track of how many ticks rdtsc takes (for 2 calls, start and end)
static uint64_t RDTSC_COST{};
static uint64_t CLOCK_GETTIME_COST{};

// Local functions
static inline uint64_t get_nsecs(void)
{
    static struct timespec ts{}; // NOTE: not thread-safe! but decreases 2ns...
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    //return ts.tv_sec*NSEC_PER_SEC+ts.tv_nsec; // this costs 2ns...
    return (ts.tv_sec<<30) - 73741824 + ts.tv_nsec; // decreases another 1ns...
}

static inline uint64_t rdtsc()
{
  unsigned int lo, hi;
  asm volatile (
     "rdtscp \n"
   : "=a"(lo), "=d"(hi) /* outputs */
   : "a"(0)             /* inputs */
   : "%ebx", "%ecx");     /* clobbers*/
  return (((uint64_t)lo) | (((uint64_t)hi) << 32));
}

// benchmark macro
#define measure_time(best, utility, code) {\
          uint64_t mtt_start, mtt_end; \
          for (auto i = 0; i < benchmark::ITERATIONS; ++i) { \
	          mtt_start = utility; \
              code; \
	          mtt_end = utility; \
          	  uint64_t mtt_delta = mtt_end - mtt_start; \
	          if (mtt_delta < best) best = mtt_delta; \
          } \
      }

static inline struct timespec
TimeSpecDiff(struct timespec *ts1, struct timespec *ts2)
{
    struct timespec ts;
    ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
    ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
    if (ts.tv_nsec < 0) {
      --ts.tv_sec;
      ts.tv_nsec += NSEC_PER_SEC;
    }
    return ts;
}

// start - signal initialization
void event_handler(int signum)
{
    static struct timespec begints, endts;
    static uint64_t begin, end;
    static uint64_t cnt = 0;
    if (cnt == 0)
    { // First alarm, there will be LOOPS - 1 alarms after
        ::clock_gettime(CLOCK_MONOTONIC, &begints);
        begin = rdtsc();
    }
    ++cnt;
    if (cnt >= TIMER_LOOPS)
    { // after LOOPS alarms, we will stop and see...
        // Disarming the  timer which was generating the SIGALRM signal
        struct itimerval timer{};
        ::setitimer(ITIMER_REAL, &timer, NULL);

        // Handle exit
        // Update timers
        end = rdtsc();
        ::clock_gettime(CLOCK_MONOTONIC, &endts);

        // Now find the ratio of RDTSC per our total time spent in NS, to see
        // how many RDTSC counts equal one nanosec
        struct timespec timer_clock_gettime_total_diff = TimeSpecDiff(&endts, &begints);
        uint64_t total_nsec_elapsed = timer_clock_gettime_total_diff.tv_sec * NSEC_PER_SEC + timer_clock_gettime_total_diff.tv_nsec;
        double total_rdtsc_elapsed = (double)end-(double)begin;
        double tickPerNS = total_rdtsc_elapsed / total_nsec_elapsed;
        g_ticks_per_nanosec[CalibrationType::SIGNAL] = tickPerNS;

        printf("<SIGNAL> RDTSC Ticks per nanosec: %.02f\n", g_ticks_per_nanosec[CalibrationType::SIGNAL]);

        double avg_nsec = (double)(total_nsec_elapsed / cnt); // in nanoseconds
        double avg_tick = (double)(total_rdtsc_elapsed / cnt); // in ticks
        uint64_t HZ = (uint64_t)(1.0 / (avg_nsec/NSEC_PER_SEC));

        printf("kernel timer interrupt frequency is approx. %lu Hz, Avg. Seconds per Signal: %.9f, Avg. Ticks per Signal (per %d us): %.9f, Count: %lu", HZ, (avg_nsec/NSEC_PER_SEC), TIMER_INTERVAL_IN_US, avg_tick, cnt);
        if (HZ >= (int)(1.0 / ((double)(TIMER_INTERVAL_IN_US) / USEC_PER_SEC)))
        {
            printf(" or higher");
        }
        printf("\n");

        waiting_for_signal = false;
    }
}

void calibrate_ticks_with_sigalarm()
{
    // In 100 usec, our signal event handler should be called, and then coninuously every 100us after
    struct sigaction sa;
    ::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &event_handler;
    ::sigaction(SIGALRM, &sa, NULL);

    // Setting timer, which generates a SIGALRM signal
    struct itimerval timer{};
    timer.it_value.tv_usec = TIMER_INTERVAL_IN_US; // alarm value
    timer.it_interval.tv_usec = TIMER_INTERVAL_IN_US; // resets to this every time
    ::clock_gettime(CLOCK_MONOTONIC, &timer_prev);
    timer_rdtsc_prev = rdtsc();
    ::setitimer(ITIMER_REAL, &timer, NULL);
    while (waiting_for_signal)
    {
        ::usleep(TIMER_INTERVAL_IN_US*TIMER_LOOPS);
    };

	{   // calculating how many ticks it costs to actually call rdtscp 2x
	    // (start, end) of measurement, so we can remove it from future 
	    // benchmark costs
		uint64_t best = ~0UL;
    	measure_time(best, rdtsc(), 0);
    	RDTSC_COST = best;
	}

    {   // calculating how many ticks it costs to actually call clock_gettime 2x
	    // (start, end) of measurement, so we can remove it from future 
	    // benchmark costs
	    uint64_t best = ~0UL;
        measure_time(best, get_nsecs(), 0);
        CLOCK_GETTIME_COST = best;
	}
    printf("rdtsc: %lu ticks, %.02f ns, clock_gettime: %lu ns\n", RDTSC_COST, get_nanos_from_ticks(RDTSC_COST), CLOCK_GETTIME_COST);
}
// force insta-initialization pre-main
static bool initialized_timer = (calibrate_ticks_with_sigalarm(), true);
// end  - signal initialization

/*
// start - loop initialization
static inline void calibrate_ticks_with_loop()
{
    struct timespec begints, endts;
    ::clock_gettime(CLOCK_MONOTONIC, &begints);
    auto begin = rdtsc();
    volatile double x{};
    for (uint64_t i = 0; i < CALIBRATE_LOOPS; i++) { x*=i; }// must be CPU intensive
    auto end = rdtsc();
    ::clock_gettime(CLOCK_MONOTONIC, &endts);
    struct timespec ndiff = TimeSpecDiff(&endts, &begints);
    uint64_t nsecElapsed = ndiff.tv_sec * 1000000000LL + ndiff.tv_nsec;
    double tickPerNS = (((double)(end-begin)/CALIBRATE_LOOPS) / (double)(ndiff.tv_nsec/CALIBRATE_LOOPS));
    g_ticks_per_nanosec[CalibrationType::LOOP] = tickPerNS;
    printf("  <LOOP> RDTSC Ticks per nanosec: %.02f\n", g_ticks_per_nanosec[CalibrationType::LOOP]);
}
// force insta-initialization pre-main? (TODO validate that this initializes in static initialization time)
static bool initialized_rdtsc = (calibrate_ticks_with_loop(), true);
// end - loop initialization
*/

/** benchmark a function that expects a variable set of arguments in the following format:

    #include <cstdarg> // va_list
    int func(const std::string& label, args);

    Or

    template <typename... Args>
    int func(const std::string& label, Args... args);
*/
template<typename TF, typename ... Args>
static inline void benchmark(const char* label, TF&& func, Args... args)
{
    uint16_t cpu = sched_getcpu();
    uint64_t r_best{~0UL}, t_best{~0UL};
	uint64_t t_start = get_nsecs();
	// std::forward adds 4 ticks in debug mode...
    measure_time(r_best, rdtsc(), func(std::forward<Args>(args) ...));
    measure_time(t_best, get_nsecs(), func(std::forward<Args>(args) ...));
	/**
	    t_total_delta provided to show what goes into measuring time at start / function / time at end, and even then we are still off by 100ns
	    Formula is: (((total - (cost*loops))/loops))/2 - 100ns
            1) total time minus
            2) cost of measurement (RDTSC + CLOCK_GETTIME) * LOOPS == Cost of func(val) 
            -----
            3) Cost of func(val) now divided by how many loops we did to find value in nsec
            4) then, divided by 2 (because we did it once per type of measurement (RDTSC and CLOCK_GETTIME)
            5) then, for some reason, we are still off by 100ns in this machine...
		uint64_t t_total_delta = (((get_nsecs() - t_start)-((get_nanos_from_ticks(RDTSC_COST)+CLOCK_GETTIME_COST)*ITERATIONS)-CLOCK_GETTIME_COST)/(ITERATIONS<<1));
    */
	uint64_t r_delta = r_best - RDTSC_COST;
	uint64_t t_delta = t_best - CLOCK_GETTIME_COST; // NOTE: We are not using t_delta, as it has less definition (it seems to come as a ceil(r_delta))...
	printf("%8lu ticks; (%0.2f) ns per invocation; %17s on cpu (%02d)\n", r_delta, get_nanos_from_ticks(r_delta), label, cpu);
}

} // namespace benchmark

/**
  * [0] Code Region - RDTSC START
  *
  * Iterations:        1
  * Instructions:      13
  * Total Cycles:      108
  * Total uOps:        13

  * Dispatch Width:    4
  * uOps Per Cycle:    0.12
  * IPC:               0.12
  * Block RThroughput: 3.3

  * processor       : 0     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 1967.746
  * processor       : 1     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 1970.595
  * processor       : 2     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 1926.369
  * processor       : 3     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 2056.479
  * processor       : 4     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 2056.750
  * processor       : 5     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 2062.905
  * processor       : 6     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 2061.177
  * processor       : 7     model name      : Intel(R) Core(TM) i7-3612QM CPU @ 2.10GHz     cpu MHz         : 2035.561

  * # taskset -c 5 ./a.out
  * Signal Ticks per nanosec: 2.09519
  * kernel timer interrupt frequency is approx. 9989 Hz, Avg. Seconds per Signal: 0.000100109, Count: 10000
  * Loops  Ticks per nanosec: 2.23009
  * CPU: 5 Ticks: 36 : 17.1822ns,
  */
//int main(int argc, char **argv)
//{
//    uint64_t cpu = sched_getcpu();
//    __asm volatile("# LLVM-MCA-BEGIN RDTSC START");
//    uint64_t start = rdtsc();
//    uint64_t ticks = rdtsc() - start;
//    __asm volatile("# LLVM-MCA-END");
//
//    std::cout << "CPU: " << cpu << " Ticks: " << ticks << " : " << get_nanos_from_ticks(ticks) << "ns, " << std::endl;
//}
