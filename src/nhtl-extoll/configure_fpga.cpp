#include <iostream>

#include "nhtl-extoll/configure_fpga.h"

namespace nhtl_extoll {

void configure_fpga(Endpoint& connection, PartnerHostConfiguration config)
{
	RegisterFile rf{connection};
	rf.write<HostEndpoint>(
	    {config.local_node, config.protection_domain_id, config.vpid, config.mode});
	rf.write<ConfigResponse>({config.config_put_address});

	rf.write<HicannBufferStart>({config.hicann.start_address});
	rf.write<HicannBufferSize>({config.hicann.capacity});
	rf.write<HicannBufferFullThreshold>({config.hicann.threshold});
	rf.write<HicannNotificationBehaviour>({config.hicann.timeout, config.hicann.frequency});
	if (config.hicann.reset_counter) {
		rf.write<HicannBufferCounterReset>({true});
	}

	// Start of trace buffer configuration
	// Remove when trace ring buffer is removed from FPGA
	rf.write<TraceBufferStart>({config.trace.start_address});
	rf.write<TraceBufferSize>({config.trace.capacity});
	rf.write<TraceBufferFullThreshold>({config.trace.threshold});
	rf.write<TraceNotificationBehaviour>({config.trace.timeout, config.trace.frequency});
	if (config.trace.reset_counter) {
		rf.write<TraceBufferCounterReset>({true});
	}

	rf.write<TraceBufferInit>({true});
	// End of trace buffer configuration

	rf.write<HicannBufferInit>({true});
	connection.ring_buffer.reset();
}

void configure_fpga(Endpoint& connection)
{
	const auto& ring_buffer = connection.ring_buffer;
	const auto& trace_ring_buffer = connection.trace_ring_buffer;

	PartnerHostConfiguration config{
	    rma2_get_nodeid(connection.get_rma_port()),
	    0,
	    connection.get_rma_vpid(),
	    0b100,
	    connection.buffer.address(),
	    {ring_buffer.address(0), uint32_t(ring_buffer.size() * sizeof(uint64_t)), 0x7c0, false,
	     0x100, uint32_t(ring_buffer.size() / 62 - 8)},
	    {trace_ring_buffer.address(0), uint32_t(ring_buffer.size() * sizeof(uint64_t)), 0x7c0,
	     false, 0x100, uint32_t(ring_buffer.size() / 62 - 8)}};

	configure_fpga(connection, config);
}

// HostEndpoint
HostEndpoint::HostEndpoint(
    uint32_t node_id_, uint32_t protection_domain_, uint32_t vpid_, uint32_t mode_)
{
	raw |= (uint64_t(node_id_) & 0xffff) << 0;
	raw |= (uint64_t(protection_domain_) & 0xffff) << 16;
	raw |= (uint64_t(vpid_) & 0x3ff) << 32;
	raw |= (uint64_t(mode_) & 0x3f) << 42;
}

uint32_t HostEndpoint::node_id() const
{
	return uint32_t(raw >> 0 & 0xffff);
}

uint32_t HostEndpoint::protection_domain() const
{
	return uint32_t(raw >> 16 & 0xffff);
}

uint32_t HostEndpoint::vpid() const
{
	return uint32_t(raw >> 32 & 0x3ff);
}

uint32_t HostEndpoint::mode() const
{
	return uint32_t(raw >> 42 & 0x3f);
}

void HostEndpoint::node_id(uint32_t value)
{
	raw &= ~(0xffffull << 0);
	raw |= (uint64_t(value) & 0xffff) << 0;
}

void HostEndpoint::protection_domain(uint32_t value)
{
	raw &= ~(0xffffull << 16);
	raw |= (uint64_t(value) & 0xffff) << 16;
}

void HostEndpoint::vpid(uint32_t value)
{
	raw &= ~(0x3ffull << 32);
	raw |= (uint64_t(value) & 0x3ff) << 32;
}

void HostEndpoint::mode(uint32_t value)
{
	raw &= ~(0x3full << 42);
	raw |= (uint64_t(value) & 0x3f) << 42;
}

// ConfigResponse
uint64_t ConfigResponse::address() const
{
	return raw;
}

void ConfigResponse::address(uint64_t value)
{
	raw = value;
}

// HicannBufferStart
uint64_t HicannBufferStart::data() const
{
	return raw;
}

void HicannBufferStart::data(uint64_t value)
{
	raw = value;
}

// HicannBufferSize
HicannBufferSize::HicannBufferSize(uint32_t data_)
{
	raw = uint64_t(data_);
}

uint32_t HicannBufferSize::data() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

void HicannBufferSize::data(uint32_t value)
{
	raw = uint64_t(value);
}

// HicannBufferFullThreshold
HicannBufferFullThreshold::HicannBufferFullThreshold(uint32_t data_)
{
	raw |= uint64_t(data_);
}

uint32_t HicannBufferFullThreshold::data() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

void HicannBufferFullThreshold::data(uint32_t value)
{
	raw = uint64_t(value);
}

// HicannNotificationBehaviour
HicannNotificationBehaviour::HicannNotificationBehaviour(uint32_t timeout_, uint32_t frequency_)
{
	raw |= uint64_t(timeout_) << 0;
	raw |= uint64_t(frequency_) << 32;
}

uint32_t HicannNotificationBehaviour::timeout() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

uint32_t HicannNotificationBehaviour::frequency() const
{
	return uint32_t(raw >> 32 & 0xffffffff);
}

void HicannNotificationBehaviour::timeout(uint32_t value)
{
	raw = uint64_t(value) << 0;
}

void HicannNotificationBehaviour::frequency(uint32_t value)
{
	raw = uint64_t(value) << 32;
}

// HicannBufferInit
HicannBufferInit::HicannBufferInit(bool start_)
{
	raw = uint64_t(start_) & 0x1;
}

bool HicannBufferInit::start() const
{
	return bool(raw >> 0 & 0x1);
}

void HicannBufferInit::start(bool value)
{
	raw = uint64_t(value) & 0x1;
}

// HicannBufferCounterReset
HicannBufferCounterReset::HicannBufferCounterReset(bool reset_)
{
	raw |= (uint64_t(reset_) & 0x1) << 0;
}

bool HicannBufferCounterReset::reset() const
{
	return bool(raw >> 0 & 0x1);
}

void HicannBufferCounterReset::reset(bool value)
{
	raw &= ~(0x1ull << 0);
	raw |= (uint64_t(value) & 0x1) << 0;
}

// TraceBufferStart
uint64_t TraceBufferStart::data() const
{
	return raw;
}

void TraceBufferStart::data(uint64_t value)
{
	raw = value;
}

// TraceBufferSize
TraceBufferSize::TraceBufferSize(uint32_t data_)
{
	raw = uint64_t(data_);
}

uint32_t TraceBufferSize::data() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

void TraceBufferSize::data(uint32_t value)
{
	raw = uint64_t(value);
}

// TraceBufferFullThreshold
TraceBufferFullThreshold::TraceBufferFullThreshold(uint32_t data_)
{
	raw |= uint64_t(data_);
}

uint32_t TraceBufferFullThreshold::data() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

void TraceBufferFullThreshold::data(uint32_t value)
{
	raw = uint64_t(value);
}

// TraceNotificationBehaviour
TraceNotificationBehaviour::TraceNotificationBehaviour(uint32_t timeout_, uint32_t frequency_)
{
	raw |= uint64_t(timeout_) << 0;
	raw |= uint64_t(frequency_) << 32;
}

uint32_t TraceNotificationBehaviour::timeout() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

uint32_t TraceNotificationBehaviour::frequency() const
{
	return uint32_t(raw >> 32 & 0xffffffff);
}

void TraceNotificationBehaviour::timeout(uint32_t value)
{
	raw = uint64_t(value) << 0;
}

void TraceNotificationBehaviour::frequency(uint32_t value)
{
	raw = uint64_t(value) << 32;
}

// TraceBufferInit
TraceBufferInit::TraceBufferInit(bool start_)
{
	raw = uint64_t(start_) & 0x1;
}

bool TraceBufferInit::start() const
{
	return bool(raw >> 0 & 0x1);
}

void TraceBufferInit::start(bool value)
{
	raw = uint64_t(value) & 0x1;
}

// TraceBufferCounterReset
TraceBufferCounterReset::TraceBufferCounterReset(bool reset_)
{
	raw |= (uint64_t(reset_) & 0x1) << 0;
}

bool TraceBufferCounterReset::reset() const
{
	return bool(raw >> 0 & 0x1);
}

void TraceBufferCounterReset::reset(bool value)
{
	raw &= ~(0x1ull << 0);
	raw |= (uint64_t(value) & 0x1) << 0;
}

} // namespace nhtl_extoll
