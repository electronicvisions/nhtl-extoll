#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "rma2.h"

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
	NotificationPoller(RMA2_Port p);
	~NotificationPoller();

	bool consume_response(std::chrono::milliseconds);
	uint64_t consume_packets(std::chrono::milliseconds);
};

} // namespace nhtl_extoll
