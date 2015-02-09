#ifndef _CM_STOPWATCH
#define _CM_STOPWATCH 1

#include <chrono>

namespace cm {

using namespace std::chrono;

/**
 * @class stopwatch
 * @brief A C++11 stopwatch based on std::chrono
 *
 * @tparam Clock
 */
template <class Clock = system_clock>

	//TODO: static_assert for testing template types

	class stopwatch {

		public:

			typedef typename Clock::time_point        time_point;
			typedef typename Clock::duration          duration;

			stopwatch(duration &elapsed, bool stopped = false) :
				_elapsed(elapsed), _stopped(stopped) {

					if (! stopped)
						start();
				}

			inline const time_point & begin() const {
				return _begin;
			}

			inline void start() {
				_stopped = false;
				_begin = Clock::now();
				_split *= 0;
			}

			inline void stop() {
				if (is_stopped()) {
					return;
				} else {
					_split = (Clock::now() - _begin);
					_elapsed += _split;
					_split *= 0;
				}
				_stopped = true;
			}

			inline void restart() {
				stop();
				start();
			}

			inline void reset() {
				_begin = Clock::now();
				_split *= 0;
				_elapsed *= 0;
			}

			inline constexpr bool is_stopped() const {
				return _stopped;
			}

			// Cast operator to duration
			inline constexpr operator duration() const {
				if ( is_stopped())
					return _elapsed + _split;
				else
					return _elapsed + (Clock::now() - _begin );
			}
			// Elapsed count for any duration rep
			template<typename U>
				typename U::rep elapsed() const {
					return duration_cast<U>(
							static_cast<typename Clock::duration>(*this)
							).count();
				}

			// Helper methods to get current duration
			inline constexpr duration until_now() const {
				return static_cast<typename Clock::duration>(*this);
			}

			virtual  ~stopwatch() {
				stop();
			}

		private:
			duration & _elapsed;
			duration   _split;
			bool       _stopped;
			time_point _begin;

	};

// Helper types

typedef stopwatch<>             high_resolution_stopwatch;
typedef stopwatch<system_clock> system_stopwatch;
typedef stopwatch<steady_clock> steady_stopwatch;

using hires_stopwatch           = high_resolution_stopwatch;
using system_stopwatch          = system_stopwatch;
using steady_stopwatch          = steady_stopwatch;

// Helper functions for duration conversion

double to_secs(stopwatch<>::duration const& d) {
	return duration_cast<duration<double>>(d).count();
}

long long to_ms(stopwatch<>::duration const& d) {
	return duration_cast<milliseconds>(d).count();
}

long long to_us(stopwatch<>::duration const& d) {
	return duration_cast<microseconds>(d).count();
}

long long to_ns(stopwatch<>::duration const& d) {
	return duration_cast<nanoseconds>(d).count();
}

// More specific "alias" fo helper functions

template <typename... Args>
auto to_seconds(Args&&... args) -> decltype(to_secs(std::forward<Args>(args)...)) {
	return to_secs(std::forward<Args>(args)...);
}

template <typename... Args>
auto to_microseconds(Args&&... args) -> decltype(to_us(std::forward<Args>(args)...)) {
	return to_us(std::forward<Args>(args)...);
}

template <typename... Args>
auto to_milliseconds(Args&&... args) -> decltype(to_ms(std::forward<Args>(args)...)) {
	return to_ms(std::forward<Args>(args)...);
}

template <typename... Args>
auto to_nanoseconds(Args&&... args) -> decltype(to_ns(std::forward<Args>(args)...)) {
	return to_ns(std::forward<Args>(args)...);
}


} // namespace cm

#endif // _CM_STOPWATCH
