#include "nhtl-extoll/buffer.h"
#include "nhtl-extoll/exception.h"
#include "nhtl-extoll/throw_on_error.h"
#include <cassert>
#include <chrono>

namespace nhtl_extoll {

PhysicalBuffer::PhysicalBuffer(RMA2_Port port, size_t pages) :
    m_port(port), m_data(pages * page_size / sizeof(uint64_t))
{
	if (pages == 0) {
		throw std::runtime_error("Page number must be positive!");
	}
	if (sysconf(_SC_PAGESIZE) != 4096) {
		throw std::runtime_error("System page size not 4096!");
	}

	RMA2_ERROR status =
	    rma2_register(m_port, m_data.data(), m_data.size() * sizeof(uint64_t), &m_region);
	throw_on_error<FailedToRegisterRegion>(status);
}

PhysicalBuffer::~PhysicalBuffer()
{
	rma2_unregister(m_port, m_region);
}

RMA2_Region* PhysicalBuffer::region() const
{
	return m_region;
}

uint64_t PhysicalBuffer::operator[](size_t position) const
{
	return m_data[position];
}

uint64_t& PhysicalBuffer::operator[](size_t position)
{
	return m_data[position];
}

size_t PhysicalBuffer::size() const
{
	return m_data.size();
}

RMA2_NLA PhysicalBuffer::address(size_t offset) const
{
	RMA2_NLA nla;
	rma2_get_nla(m_region, offset, &nla);
	return nla;
}

RingBuffer::RingBuffer(RMA2_Port port, RMA2_Handle handle, size_t pages, NotificationPoller& p) :
    PhysicalBuffer(port, pages), m_handle(handle), m_poller(p)
{}

RingBuffer::~RingBuffer()
{
	while (receive(false))
		;
	m_read_index += m_readable_words;
	m_readable_words = 0;
	notify();
}

uint64_t RingBuffer::get()
{
	while (m_readable_words == 0) {
		receive(true);
	}
	m_read_index %= size();
	uint64_t read = m_data[m_read_index++];
	++m_read_words;
	--m_readable_words;

	if (m_read_words > 10) {
		notify();
	}
	return read;
}

void RingBuffer::notify()
{
	while (m_read_words > 0) {
		auto words_to_notify = std::min(m_read_words, 0x1ffffffful);

		// Identifier for the (hicann) ring buffer
		uint64_t type = 10779;
		uint64_t payload = (type << 48u) | words_to_notify;
		rma2_post_notification(
		    m_port, m_handle, 0, payload, RMA2_NO_NOTIFICATION, RMA2_CMD_DEFAULT);

		m_read_words -= words_to_notify;
	}
}

bool RingBuffer::receive(bool throw_on_timeout)
{
	uint64_t packets = m_poller.consume_packets(std::chrono::milliseconds(20));
	m_readable_words += packets;

	if (packets == 0 && throw_on_timeout) {
		throw std::runtime_error("Hicann response timed out!");
	}

	return packets != 0;
}

void RingBuffer::reset()
{
	m_read_index = 0;
	m_readable_words = 0;
	m_read_words = 0;
}

} // namespace nhtl_extoll
