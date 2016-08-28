#ifndef process_timing_hpp
#define process_timing_hpp

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace timings {

    // "overloading" std ratio comparisons: R1 op R2 <==> R1::num/R1::den op R2::num/R2::den <==> R1::num*R2::den op R2::num*R1::den

    template <class R1, class R2>
    using ratio_equal = std::ratio_equal<R1, R2>;

    template <class R1, class R2>
    using ratio_not_equal = std::ratio_not_equal<R1, R2>;

    template <class R1, class R2>
    struct ratio_less : std::integral_constant<bool, R1::num * R2::den < R2::num * R1::den> { };

    template <class R1, class R2>
    struct ratio_less_equal : std::integral_constant<bool, !ratio_less<R2, R1>::value> { };

    template <class R1, class R2>
    struct ratio_greater : std::integral_constant<bool, ratio_less<R2, R1>::value> { };

    template <class R1, class R2>
    struct ratio_greater_equal : std::integral_constant<bool, !ratio_less<R1, R2>::value> { };



    /**
     *    @brief days represents the time in number of days.
     */
    using   days    =   std::chrono::duration<std::chrono::hours::rep, std::ratio<86400,1>>;



    /**
     *    @brief The TimeElements struct represents a time as a set of standard elements, like seconds.
     */
    class TimeElements {
    public:
        days                        d;      ///< Days.
        std::chrono::hours          h;      ///< Hours.
        std::chrono::minutes        m;      ///< Minutes.
        std::chrono::seconds        s;      ///< Seconds.
        std::chrono::milliseconds   ms;     ///< Milliseconds.
        std::chrono::microseconds   us;     ///< Microseconds.
        std::chrono::nanoseconds    ns;     ///< Nanoseconds.
    };



    /// Main class, providing methods for taking timings.
    class ProcessTiming {
    public:
        /**
         *    @brief SplitTimeElements extracts elements from time.
         *    @param duration The time to split.
         *    @param timeElements Elements to be extracted.
         */
        template < typename Rep, typename Period >
        static void SplitTimeElements(std::chrono::duration<Rep,Period> duration, TimeElements &timeElements)
        {
            timeElements.d = std::chrono::duration_cast<days>(duration);
            duration -= timeElements.d;
            timeElements.h = std::chrono::duration_cast<std::chrono::hours>(duration);
            duration -= timeElements.h;
            timeElements.m = std::chrono::duration_cast<std::chrono::minutes>(duration);
            duration -= timeElements.m;
            timeElements.s = std::chrono::duration_cast<std::chrono::seconds>(duration);
            duration -= timeElements.s;
            timeElements.ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            duration -= timeElements.ms;
            timeElements.us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            duration -= timeElements.us;
            timeElements.ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
        }

        /**
         *    @brief TimeElementsToString converts a time into a string representation.
         *    @param timeElements The time elements to convert.
         *    @param timeStr The output string.
         */
        template < class Period >
        static void TimeElementsToString(const TimeElements &timeElements, std::string &timeStr)
        {
            std::ostringstream stream;
            bool activate = false;
            if (timeElements.d.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, days::period>::value && activate)
                stream << timeElements.d.count() << "d.";
            if (timeElements.h.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::hours::period>::value && activate)
                stream << std::setw(2) << std::setfill('0') << timeElements.h.count() << "h.";
            if (timeElements.m.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::minutes::period>::value && activate)
                stream << std::setw(2) << std::setfill('0') << timeElements.m.count() << "m.";
            if (timeElements.s.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::seconds::period>::value && activate)
                stream << std::setw(2) << std::setfill('0') << timeElements.s.count() << "s.";
            if (timeElements.ms.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::milliseconds::period>::value && activate)
                stream << std::setw(3) << std::setfill('0') << timeElements.ms.count() << "ms.";
            if (timeElements.us.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::microseconds::period>::value && activate)
                stream << std::setw(3) << std::setfill('0') << timeElements.us.count() << "us.";
            if (timeElements.ns.count() > 0)
                activate = true;
            if (ratio_less_equal<Period, std::chrono::nanoseconds::period>::value && activate)
                stream << std::setw(3) << std::setfill('0') << timeElements.ns.count() << "ns.";
            timeStr = stream.str();
        }

        /**
         *    @brief TimeToString converts duration into a string representation.
         *    @param duration The time to convert.
         *    @param timeStr The output string.
         */
        template < typename Rep, typename Period >
        static void TimeToString(const std::chrono::duration<Rep,Period> &duration, std::string &timeStr)
        {
            TimeElements timeElements;
            SplitTimeElements(duration, timeElements);
            TimeElementsToString<Period>(timeElements, timeStr);
        }



        /**
         *    @brief Default constructor.
         */
        ProcessTiming()
        {
            start();
        }

        /**
         *    @brief Initialize the counter.
         */
        inline void start()
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _startTime = std::chrono::steady_clock::now();
            _endTime = _startTime;
            _ongoing = true;
        }

        /**
         *    @brief Terminate the counter.
         */
        inline void stop()
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _endTime = std::chrono::steady_clock::now();
            _ongoing = false;
        }

        /**
         *    @brief Return the initial time point.
         */
        inline std::chrono::steady_clock::time_point getStartTime() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _startTime;
        }

        /**
         *    @brief Return the final time point.
         */
        inline std::chrono::steady_clock::time_point getEndTime() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            std::chrono::steady_clock::time_point endTime = _endTime;
            if (_ongoing)
                endTime = std::chrono::steady_clock::now();
            return endTime;
        }

        /**
         *    @brief Returns if it's counting or not (a.k.a. true if start() has been called and stop() not yet).
         */
        inline bool isRunning() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _ongoing;
        }

        /**
         *    @brief Return how much time has been elapsed since the start.
         */
        template < typename Rep = std::chrono::nanoseconds::rep, typename Period = std::chrono::nanoseconds::period >
        inline std::chrono::duration<Rep,Period> elapsed() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            std::chrono::duration<Rep,Period> diff;
            if (_ongoing)
                diff = std::chrono::duration_cast<std::chrono::duration<Rep,Period>>(std::chrono::steady_clock::now() - _startTime);
            else
                diff = std::chrono::duration_cast<std::chrono::duration<Rep,Period>>(_endTime - _startTime);
            return diff;
        }

        /**
         *    @brief Return a string representation of the elapsed time from the start.
         */
        template < typename Rep = std::chrono::nanoseconds::rep, typename Period = std::chrono::nanoseconds::period >
        inline std::string to_string() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            std::chrono::duration<Rep,Period> duration = elapsed<Rep,Period>();
            std::string timeStr;
            TimeToString<Rep,Period>(duration, timeStr);
            return timeStr;
        }

    private:
        mutable std::recursive_mutex            _mutex;         ///< Ensures the mutual exclusion (in multi-thread processes).
        bool                                    _ongoing;       ///< Tells if the counter is counting.
        std::chrono::steady_clock::time_point   _startTime;     ///< The initial time point.
        std::chrono::steady_clock::time_point   _endTime;       ///< The final time point.
    };

}

#endif // process_timing_hpp
