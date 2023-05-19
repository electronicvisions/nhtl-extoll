#pragma once
#include "hate/visibility.h"
#include "rma2.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sched.h>
#include <thread>

namespace nhtl_extoll {

class NotificationPoller
{
private:
	RMA2_Port m_port;
	uint64_t m_packets{0};
	uint64_t m_notifications{0};

	std::atomic<bool> m_running;
	std::thread m_thread;
	std::mutex m_mutex;
	std::condition_variable m_cv;

	void poll_notifications();

public:
	NotificationPoller(RMA2_Port p) SYMBOL_VISIBLE;
	~NotificationPoller() SYMBOL_VISIBLE;

	bool consume_response(std::chrono::milliseconds) SYMBOL_VISIBLE;
	uint64_t consume_packets(std::chrono::milliseconds) SYMBOL_VISIBLE;

	// Used to restrict process to single CPU to avoid notification latency issues.
	cpu_set_t cpu;
};

} // namespace nhtl_extoll
