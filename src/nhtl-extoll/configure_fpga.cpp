#include <iostream>

#include "nhtl-extoll/configure_fpga.h"

namespace nhtl_extoll {

void configure_fpga(Endpoint& connection, PartnerHostConfiguration config)
{
	connection.rra_write<HostEndpoint>(
	    {config.local_node, config.protection_domain_id, config.vpid, config.mode});
	connection.rra_write<ConfigResponse>({config.config_put_address});

	connection.rra_write<HicannBufferStart>({config.hicann.start_address});
	connection.rra_write<HicannBufferSize>({config.hicann.capacity});
	connection.rra_write<HicannBufferFullThreshold>({config.hicann.threshold});
	connection.rra_write<HicannNotificationBehaviour>(
	    {config.hicann.timeout, config.hicann.frequency});
	if (config.hicann.reset_counter) {
		connection.rra_write<HicannBufferCounterReset>({true});
	}

	// Start of trace buffer configuration
	// Remove when trace ring buffer is removed from FPGA
	connection.rra_write<TraceBufferStart>({config.trace.start_address});
	connection.rra_write<TraceBufferSize>({config.trace.capacity});
	connection.rra_write<TraceBufferFullThreshold>({config.trace.threshold});
	connection.rra_write<TraceNotificationBehaviour>(
	    {config.trace.timeout, config.trace.frequency});
	if (config.trace.reset_counter) {
		connection.rra_write<TraceBufferCounterReset>({true});
	}

	connection.rra_write<TraceBufferInit>({true});
	// End of trace buffer configuration

	connection.rra_write<HicannBufferInit>({true});
	connection.hicann_ring_buffer.reset();
	connection.trace_ring_buffer.reset();

	connection.rra_write<HicannTracePktClosure>({config.hicann_trace_pkt_closure});

	Info info = connection.rra_read<Info>();
	info.ndid(uint16_t(connection.get_node()));
	connection.rra_write<Info>(info);
}

void configure_fpga(Endpoint& connection)
{
	const auto& hicann_ring_buffer = connection.hicann_ring_buffer;
	const auto& trace_ring_buffer = connection.trace_ring_buffer;

	PartnerHostConfiguration config{
	    rma2_get_nodeid(connection.get_rma_port()),
	    0,
	    connection.get_rma_vpid(),
	    0b100,
	    connection.buffer.response_address(),
	    {hicann_ring_buffer.address(0), static_cast<uint32_t>(hicann_ring_buffer.size_bt), 0x7c0,
	     false, 0x100, static_cast<uint32_t>(hicann_ring_buffer.size_qw / 62 - 8)},
	    {trace_ring_buffer.address(0), static_cast<uint32_t>(trace_ring_buffer.size_bt), 0x7c0,
	     false, 0x100, static_cast<uint32_t>(trace_ring_buffer.size_qw / 62 - 8)},
	    512};

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

// HicannTracePktClosure
HicannTracePktClosure::HicannTracePktClosure(uint32_t timeout_)
{
	raw |= uint64_t(timeout_) << 0;
}

uint32_t HicannTracePktClosure::timeout() const
{
	return uint32_t(raw >> 0 & 0xffffffff);
}

void HicannTracePktClosure::timeout(uint32_t value)
{
	raw = uint64_t(value) << 0;
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


// Info
Info::Info(uint32_t guid_, uint16_t ndid_, uint8_t waferid_, uint8_t socketid_, uint8_t edgeid_)
{
	raw |= (uint64_t(guid_) & 0xff'ffff) << 0;
	raw |= (uint64_t(ndid_) & 0xffff) << 24;
	raw |= (uint64_t(waferid_) & 0xff) << 40;
	raw |= (uint64_t(socketid_) & 0xf) << 48;
	raw |= (uint64_t(edgeid_) & 0x3) << 52;
}

uint32_t Info::guid() const
{
	return uint32_t(raw >> 0 & 0xff'ffff);
}

void Info::guid(uint32_t value)
{
	raw |= (uint64_t(value) & 0xff'ffff) << 0;
}

uint16_t Info::ndid() const
{
	return uint16_t(raw >> 24 & 0xffff);
}

void Info::ndid(uint16_t value)
{
	raw |= (uint64_t(value) & 0xffff) << 24;
}

uint8_t Info::waferid() const
{
	return uint8_t(raw >> 40 & 0xff);
}

void Info::waferid(uint8_t value)
{
	raw |= (uint64_t(value) & 0xff) << 40;
}

uint8_t Info::socketid() const
{
	return uint8_t(raw >> 48 & 0xf);
}

void Info::socketid(uint8_t value)
{
	raw |= (uint64_t(value) & 0xf) << 48;
}

uint8_t Info::edgeid() const
{
	return uint8_t(raw >> 52 & 0x3);
}

void Info::edgeid(uint8_t value)
{
	raw |= (uint64_t(value) & 0x3) << 52;
}

} // namespace nhtl_extoll
