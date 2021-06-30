#include "nhtl-extoll/buffer.h"
#include "nhtl-extoll/exception.h"
#include "nhtl-extoll/throw_on_error.h"
#include <cassert>

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

} // namespace nhtl_extoll
