#pragma once

#include <cstdint>
#include <vector>

#include "nhtl-extoll/notification_poller.h"
#include "rma2.h"

namespace nhtl_extoll {

/**
 *  A buffer for Remote Registerfile Access (RRA) responses. The RRA
 *  connection uses physical addresses as the registerfile only has
 *  physical addresses.
 *  FPGAs do not implement virtual addresses and would interpret them
 *  as physical addresses.
 *  However, the Tourmalet Registerfile does implement virtual addresses
 *  and would interpret them as virtual addresses instead of physical
 *  addresses, causing a translation of addresses which will fail.
 *  The size of this buffer is one page size, which has to be 4096B
 *  for the card. However, only 64 bit, i.e., one Quad Word, are used.
 */
class PhysicalBuffer
{
private:
	std::array<uint64_t, 512>* m_buffer;
	/// Use uint64_t instead of uintptr_t as Extoll uses uint64_t(=RMA2_NLA)
	uint64_t m_physical_address;

public:
	explicit PhysicalBuffer();
	/// This class is moveable as the underlying registered memory
	/// region is stable address-wise
	PhysicalBuffer(PhysicalBuffer&&) = default;
	/// This class is not copyable
	PhysicalBuffer(PhysicalBuffer const&) = delete;
	/// This class is not copy-assignable
	PhysicalBuffer& operator=(PhysicalBuffer const&) = delete;

	/// Returns the Network Logical Address (NLA) of the buffer.
	/// Note that this uses physical addresses.
	RMA2_NLA address() const;
	/// Return the quad word written at the start of the buffer.
	/// The FPGA always sends exactly one quad word as response,
	/// therefore, we only need to access the first element.
	uint64_t read() const;
};

/**
 *  A specialized memory region that acts like a ringbuffer.
 *  This class is synchronized with the ringbuffer on the remote Fpga.
 */
class RingBuffer
{
private:
	/// The network port
	RMA2_Port m_port;
	/// The user-space buffer
	std::vector<uint64_t> m_data{};
	/// The memory region registered with the driver
	RMA2_Region* m_region;
	/// Current read index
	size_t m_read_index = 0;
	/// Number of words in the buffer not yet read
	size_t m_readable_words = 0;
	/// Number of words read without notifying the FPGA
	size_t m_read_words = 0;
	/// Handle of the connection to the FPGA to notify
	RMA2_Handle m_handle = nullptr;
	/// Notification poller listening for ring buffer notifications
	NotificationPoller& m_poller;

	/// Checks with the poller if new words have arrived
	/// May throw after a timeout if no new words have arrived
	bool receive(bool throw_on_timeout);
	/// Reads the buffer at a position
	uint64_t operator[](size_t position) const;
	/// Writes to the buffer at a position
	uint64_t& operator[](size_t position);
	/// Notifies the hardware about how many quad words were read
	void notify();

public:
	/// Creates a ringbuffer from an Rma network port and handle, the
	/// number of pages and the payload type to expect
	RingBuffer(RMA2_Port port, RMA2_Handle handle, size_t pages, NotificationPoller&);
	/// Frees all resources and does a last sync with the remote Fpga
	~RingBuffer();
	/// This class is moveable as the underlying registered memory
	/// region is stable address-wise
	RingBuffer(RingBuffer&&) = default;
	/// This class is not copyable
	RingBuffer(RingBuffer const&) = delete;
	/// This class is not copy-assignable
	RingBuffer& operator=(RingBuffer const&) = delete;

	/// Page size according to RMA2 API reference manual, i.e., the
	/// libRMA code seems not page-size-generic
	static const int page_size = 4096;

	/// Blocks and reads one quad word from the buffer
	uint64_t get();
	/// Reads and discards quad words in the buffer
	void clear();
	/// Does a hard reset without notifying the hardware
	void reset();
	/// The size of the buffer in quad words
	size_t size() const;
	/// Accessor for the memory region
	RMA2_Region* region() const;
	/// The NLA of the mapped memory region with an optional offset in bytes
	RMA2_NLA address(size_t offset) const;
};
}
