#pragma once

#include <cstdint>
#include <vector>

#include "nhtl-extoll/notification_poller.h"
#include "rma2.h"

namespace nhtl_extoll {

/// A user-space buffer registered with the Rma driver.
class PhysicalBuffer
{
protected:
	/// the network port
	RMA2_Port m_port;
	/// the user-space buffer
	std::vector<uint64_t> m_data{};
	/// the registered memory region with the driver
	RMA2_Region* m_region;

public:
	/// Creates a buffer from an Rma port and the number of pages (4kB)
	explicit PhysicalBuffer(RMA2_Port port, size_t pages);
	/// Unregisters the region and frees the memory
	~PhysicalBuffer();
	/// This class is moveable as the underlying registered memory region is stable address-wise
	PhysicalBuffer(PhysicalBuffer&&) = default;
	/// This class is not copyable
	PhysicalBuffer(PhysicalBuffer const&) = delete;
	/// This class is not copy-assignable
	PhysicalBuffer& operator=(PhysicalBuffer const&) = delete;

	/// Page size according to RMA2 API reference manual, i.e., the libRMA code seems not
	/// page-size-generic
	static const int page_size = 4096;

	/// The size of the buffer in quad words
	size_t size() const;
	/// Accessor for the memory region
	RMA2_Region* region() const;
	/// The logical address of the mapped memory region with an optional offset in bytes
	RMA2_NLA address(size_t offset) const;

	/// Reads the buffer at a position
	uint64_t operator[](size_t position) const;
	/// Writes to the buffer at a position
	uint64_t& operator[](size_t position);
};

/**
 *  A specialized memory region that acts like a ringbuffer.
 *  This class is synchronized with the ringbuffer on the remote Fpga.
 */
class RingBuffer : public PhysicalBuffer
{
private:
	size_t m_read_index = 0;
	size_t m_readable_words = 0;
	size_t m_read_words = 0;
	RMA2_Handle m_handle = nullptr;
	NotificationPoller& m_poller;

	bool receive(bool throw_on_timeout);

public:
	/// Creates a ringbuffer from an Rma network port and handle, the number of pages and the
	/// payload type to expect
	RingBuffer(RMA2_Port port, RMA2_Handle handle, size_t pages, NotificationPoller&);
	/// Frees all resources and does a last sync with the remote Fpga
	~RingBuffer();

	/// Blocks and reads one quad word from the buffer
	uint64_t get();
	/// Notifies the hardware about how many quad words were read.
	/// This method is also called internally after a certain amount of read quad words
	void notify();
	/// Simulates reading all quad words in the buffer and discards the packets
	void clear();
	/// Does a hard reset without notifying the hardware.
	void reset();
};
}
