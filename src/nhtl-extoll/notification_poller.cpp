#include "nhtl-extoll/notification_poller.h"

#include <chrono>
#include <iostream>

namespace nhtl_extoll {

NotificationPoller::NotificationPoller(RMA2_Port p) :
    m_port{p},
    m_running{true},
    m_thread{&NotificationPoller::poll_notifications, this},
    m_mutex{},
    m_cv{}
{}

NotificationPoller::~NotificationPoller()
{
	m_running.store(false);
	m_thread.join();
}

void NotificationPoller::poll_notifications()
{
	using namespace std::literals::chrono_literals;

	auto wait_period = 1us;
	constexpr std::chrono::microseconds max_wait_period = 10ms;

	while (m_running) {
		RMA2_Notification* notification;
		RMA2_ERROR status = rma2_noti_probe(m_port, &notification);

		if (status == RMA2_NO_NOTI) {
			std::this_thread::sleep_for(wait_period);
			wait_period = std::min(wait_period * 2, max_wait_period);
			continue;
		} else if (status == RMA2_ERR_INV_PORT) {
			throw std::runtime_error("Invalid port in notification poller");
		}
		wait_period = 1us;

		RMA2_Class cls = rma2_noti_get_notiput_class(notification);
		uint64_t payload = rma2_noti_get_notiput_payload(notification) & 0xffffffff;
		rma2_noti_free(m_port, notification);
		{
			std::lock_guard<std::mutex> lock{m_mutex};
			switch (cls) {
				case 0xa1:
					m_packets += payload;
					break;
				case 0x0:
					++m_notifications;
					break;
				default:
					std::cerr << "Unknown notification class: " << uint16_t(cls) << "\n";
					throw std::runtime_error("Unknown notification class");
			}
		}
		m_cv.notify_all();
	}
}

bool NotificationPoller::consume_response(std::chrono::milliseconds timeout)
{
	std::unique_lock<std::mutex> lock{m_mutex};
	m_cv.wait_for(lock, timeout, [this] { return m_notifications > 0; });
	if (m_notifications > 0) {
		--m_notifications;
		return true;
	}
	return false;
}

uint64_t NotificationPoller::consume_packets(std::chrono::milliseconds timeout)
{
	std::unique_lock<std::mutex> lock{m_mutex};
	m_cv.wait_for(lock, timeout, [this] { return m_packets > 0; });
	uint64_t tmp = m_packets;
	m_packets = 0;
	return tmp;
}

} // namespace nhtl_extoll
