#include <iostream>
#include <iomanip>
#include <thread>
#include <math.h>
#include <cm/stopwatch.h>


int main(int argc, char **argv) {
	constexpr std::chrono::milliseconds second( 1000 );
	constexpr std::chrono::milliseconds half_sec( 500 );
	auto n = pow(10, 6);

	using namespace cm;

	hires_stopwatch::duration elapsed;

	// Stopwatch this scope
	{
		hires_stopwatch w(elapsed, true);

		std::cout << "* start()" << std::endl;

		w.start();
		std::cout << "+0.5" << std::endl;
		std::this_thread::sleep_for( half_sec );


		std::cout << "ns > " << std::setprecision(5) <<
			w.until_now().count() << std::endl;
		std::cout << "+0.5" << std::endl;
		std::this_thread::sleep_for( half_sec );
		std::cout << "ns > " << std::setprecision(5) <<
			w.until_now().count() << std::endl;

		std::cout << "* restart()" << std::endl;
		w.restart();

		std::cout << "+0.5" << std::endl;
		std::this_thread::sleep_for( half_sec );

		std::cout << "* stop()" << std::endl;
		w.stop();

		std::cout << "ns > " << std::setprecision(5)
			<< w.until_now().count() << std::endl;

		std::cout << "μs > " << std::setprecision(8) <<
				w.elapsed<std::chrono::microseconds>() << std::endl;

		std::cout << "ms > " << std::setprecision(8) <<
				w.elapsed<std::chrono::milliseconds>() << std::endl;


		std::cout << "ns > " << std::setprecision(8) <<
				w.elapsed<std::chrono::nanoseconds>() << std::endl;

		std::cout << "rs > " << std::setprecision(5) <<
				w.elapsed<std::chrono::duration<float>>() << std::endl;

	}

	std::cout << "---------------------------------" << std::endl;

	std::cout << "ds > " << std::setprecision(5) <<
		std::chrono::duration_cast<
			std::chrono::duration<float>
		> (elapsed).count() << std::endl;

	std::cout << "ns > " << std::setprecision(8) <<
		cm::to_ns(elapsed) << std::endl;

	std::cout << "μs > " << std::setprecision(8) <<
		cm::to_us(elapsed) << std::endl;

	std::cout << "ms > " << std::setprecision(8) <<
		cm::to_ms(elapsed) << std::endl;

	std::cout << "ds > " << std::setprecision(5) <<
		cm::to_seconds(elapsed) << std::endl;

	return 0;
}
