#include "nhtl-extoll/buffer.h"
#include "nhtl-extoll/exception.h"
#include "nhtl-extoll/throw_on_error.h"
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <pmap.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

namespace nhtl_extoll {

PhysicalBuffer::PhysicalBuffer()
{
	int const page_size = 4096;
	int ret = sysconf(_SC_PAGESIZE);
	if (ret != page_size) {
		std::cerr << "EXTOLL only supports 4kiB page size:" << std::strerror(errno);
		throw std::runtime_error("Page size must equal 4096B.");
	}

	int pmap_fd = open("/dev/extoll/pmap", O_RDWR);
	if (pmap_fd < 0) {
		std::cerr << "Opening PMAP device special file failed:" << std::strerror(errno);
		throw std::runtime_error("Failed to open /dev/extoll/pmap.");
	}

	// set type to kernel allocated memory
	ret = ioctl(pmap_fd, PMAP_IOCTL_SET_TYPE, 0);
	if (ret < 0) {
		std::cerr << "pmap ioctl PMAP_IOCTL_SET_TYPE failed:" << std::strerror(errno);
		throw std::runtime_error("Failed to set type to kernel allocated memory.");
	}

	// set size
	ret = ioctl(pmap_fd, PMAP_IOCTL_SET_SIZE, page_size);
	if (ret < 0) {
		std::cerr << "pmap ioctl PMAP_IOCTL_SET_TYPE failed:" << std::strerror(errno);
		throw std::runtime_error("Failed to set buffer size.");
	}

	// mmap the buffer
	void* map_address = mmap(
	    0,                      /* preferred start  */
	    page_size,              /* length in bytes  */
	    PROT_READ | PROT_WRITE, /* protection flags */
	    MAP_SHARED,             /* mapping flags    */
	    pmap_fd,                /* file descriptor  */
	    0                       /* offset           */
	);
	if (map_address == MAP_FAILED) {
		std::cerr << "Physcial buffer mmap failed" << std::strerror(errno);
		throw std::runtime_error("Failed to mmap buffer.");
	}
	m_buffer = new (map_address) std::array<uint64_t, 512>;

	// get physical address
	ret = ioctl(pmap_fd, PMAP_IOCTL_GET_PADDR, &m_physical_address);
	if (ret < 0) {
		std::cerr << "pmap ioctl PMAP_IOCTL_GET_PADDR failed:" << std::strerror(errno);
		throw std::runtime_error("Failed to acquire physical address of buffer.");
	}
}

RMA2_NLA PhysicalBuffer::address() const
{
	return m_physical_address;
}

uint64_t PhysicalBuffer::read() const
{
	return (*m_buffer)[0];
}

RingBuffer::RingBuffer(RMA2_Port port, RMA2_Handle handle, size_t pages, NotificationPoller& p) :
    m_port(port), m_data(pages * page_size / sizeof(uint64_t)), m_handle(handle), m_poller(p)
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

RingBuffer::~RingBuffer()
{
	while (receive(false))
		;

	m_read_index += m_readable_words;
	m_readable_words = 0;
	notify();

	rma2_unregister(m_port, m_region);
}

RMA2_Region* RingBuffer::region() const
{
	return m_region;
}

uint64_t RingBuffer::operator[](size_t position) const
{
	return m_data[position];
}

uint64_t& RingBuffer::operator[](size_t position)
{
	return m_data[position];
}

size_t RingBuffer::size() const
{
	return m_data.size();
}

RMA2_NLA RingBuffer::address(size_t offset) const
{
	RMA2_NLA nla;
	rma2_get_nla(m_region, offset, &nla);
	return nla;
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

	if (m_read_words >= 10) {
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
