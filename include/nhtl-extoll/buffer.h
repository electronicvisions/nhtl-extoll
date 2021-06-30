#pragma once

#include <cstdint>
#include <vector>

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

}
