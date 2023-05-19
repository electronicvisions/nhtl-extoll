#pragma once
#include "hate/visibility.h"
#include "nhtl-extoll/connection.h"
#include "rma2.h"
#include <cstdint>

namespace nhtl_extoll {

/**
 *  All configuration values for the partner host configuration.
 *  For a set of default parameters see the implementation.
 *  Changing these values can lead to misconfiguration of the remote Fpga
 */
struct PartnerHostConfiguration
{
	/// The node id of the local extoll node
	RMA2_Nodeid local_node;
	/// The protection domain id (currently not used)
	uint16_t protection_domain_id;
	/// The virtual process id of the communication
	RMA2_VPID vpid;
	/// The Rra mode (currently only the bit at index 2 is used to indicate translation enabled)
	uint8_t mode;

	/// The network logical address of the Fpga config response buffer
	// Should become obsolete in the future
	uint64_t config_put_address;

	/// Configuration values concerning the ring-buffers
	struct Ringbuffer
	{
		/// The start address of the memory region
		uint64_t start_address;
		/// The capacity in bytes
		uint32_t capacity;
		/// The threshold that determines the "nearly full" state on the Fpga
		/// Default: 0x7c0, i.e., 4 max. extoll packets (62 QWs) in bytes
		uint32_t threshold;
		/// A flag whether to reset the internal counters (default is false)
		bool reset_counter;

		/// The timeout until a notification is send in cycles
		/// Default: 0x100 = 256 cycles
		uint32_t timeout;
		/// The number of packets sent until a notification is sent
		/// Default: Ringbuffer size in max. extoll packets minus 8
		/// This ensures that a notification is send before the threshold is reached
		uint32_t frequency;
	};

	/// The ringbuffer configuration for the Hicann config ringbuffer
	Ringbuffer hicann;
	/// The ring buffer required for successful configuration
	/// Remove when trace ring buffer is removed from FPGA
	Ringbuffer trace;

	uint32_t hicann_trace_pkt_closure;
};

void SYMBOL_VISIBLE configure_fpga(Endpoint& connection, PartnerHostConfiguration config);
void SYMBOL_VISIBLE configure_fpga(Endpoint& connection);

/**
 *  Read-write register file HostEndpoint.
 *  Configures the Fpga with data from the local node.
 *
 *  The node id, protection domain and virtual process id all refer to the
 *  local host node. Mode must be set to `0x4` when any address is a logical
 *  address. It must be set to `0x0` when any address is a physical address.
 *  Therefor, it is not possible to mix logical and physical addresses.
 *
 *  The addresses in question are:
 *  - TraceBufferStart
 *  - HicannBufferStart
 *  - ConfigResponse
 */
struct HostEndpoint
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HostEndpoint() = default;

	/// Initialize all fields with a specific value
	HostEndpoint(uint32_t node_id_, uint32_t protection_domain_, uint32_t vpid_, uint32_t mode_)
	    SYMBOL_VISIBLE;

	/// Read the `node_id` field
	uint32_t node_id() const SYMBOL_VISIBLE;
	/// Read the `protection_domain` field
	uint32_t protection_domain() const SYMBOL_VISIBLE;
	/// Read the `vpid` field
	uint32_t vpid() const SYMBOL_VISIBLE;
	/// Read the `mode` field
	uint32_t mode() const SYMBOL_VISIBLE;


	/// Set the `node_id` field
	void node_id(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `protection_domain` field
	void protection_domain(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `vpid` field
	void vpid(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `mode` field
	void mode(uint32_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5298;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file ConfigResponse.
 *  Address of the Fpga config response packets.
 *
 *  This can be a physical or a logical address. For a logical address
 *  the HostEndpoint node must be set to `0x4`.
 */
struct ConfigResponse
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	ConfigResponse() = default;
	/// Initialize the single field with a specific value
	ConfigResponse(uint64_t value) : raw(value) {}

	/// Read the single field
	uint64_t address() const SYMBOL_VISIBLE;
	/// Set the single field
	void address(uint64_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x52a0;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannBufferStart.
 *  Address in bytes of the start of the hicann config data ringbuffer.
 *
 *  This can be a physical or a logical address. For a logical address
 *  the HostEndpoint node must be set to `0x4`.
 *
 *  For a high-level interface use the configure_fpga method.
 */
struct HicannBufferStart
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannBufferStart() = default;
	/// Initialize the single field with a specific value
	HicannBufferStart(uint64_t value) : raw(value) {}

	/// Read the single field
	uint64_t data() const SYMBOL_VISIBLE;
	/// Set the single field
	void data(uint64_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5080;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannBufferSize.
 *  The capacity of the hicann config data ringbuffer in bytes.
 *
 *  For a high-level interface use the configure_fpga method.
 */
struct HicannBufferSize
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannBufferSize() = default;

	/// Initialize all fields with a specific value
	HicannBufferSize(uint32_t data_) SYMBOL_VISIBLE;

	/// Read the `data` field
	uint32_t data() const SYMBOL_VISIBLE;
	/// Set the `data` field
	void data(uint32_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5088;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannBufferFullThreshold.
 *  The threshold that determines the full state of the hicann config data ringbuffer.
 *
 *  For a high-level interface use the configure_fpga method.
 */
struct HicannBufferFullThreshold
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannBufferFullThreshold() = default;

	/// Initialize all fields with a specific value
	HicannBufferFullThreshold(uint32_t data_) SYMBOL_VISIBLE;

	/// Read the `data` field
	uint32_t data() const SYMBOL_VISIBLE;

	/// Set the `data` field
	void data(uint32_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5090;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannNotificationBehaviour.
 *  The notification behavior of the hicann config ringbuffer.
 *
 *  Frequency is the number of packets after which the Fpga will send a payload
 *  notification to the host. Timeout specifies the number of clock cycles after
 *  which the Fpga will send a notification in case the number of packets is lower
 *  than the frequency.
 */
struct HicannNotificationBehaviour
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannNotificationBehaviour() = default;

	/// Initialize all fields with a specific value
	HicannNotificationBehaviour(uint32_t timeout_, uint32_t frequency_) SYMBOL_VISIBLE;

	/// Read the `timeout` field
	uint32_t timeout() const SYMBOL_VISIBLE;
	/// Read the `frequency` field
	uint32_t frequency() const SYMBOL_VISIBLE;

	/// Set the `timeout` field
	void timeout(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `frequency` field
	void frequency(uint32_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x52b0;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannTracePktClosure.
 *  The number of clock-cycles to wait while no AL-WR transaction appears, until
 *  the packet is sent. The value must not be smaller than ~20, as then the message-rate
 *  starts to limit transmission!
 */
struct HicannTracePktClosure
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannTracePktClosure() = default;

	/// Initialize all fields with a specific value
	HicannTracePktClosure(uint32_t timeout_) SYMBOL_VISIBLE;

	/// Read the `timeout` field
	uint32_t timeout() const SYMBOL_VISIBLE;

	/// Set the `timeout` field
	void timeout(uint32_t value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x52b8;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Read-write register file HicannBufferInit.
 *  Writing a `1` will reconfigure the hicann config data ringbuffer according to
 *  the previously written config values.
 *
 *  For a high-level interface use the configure_fpga method.
 */
struct HicannBufferInit
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannBufferInit() = default;

	/// Initialize all fields with a specific value
	HicannBufferInit(bool start_) SYMBOL_VISIBLE;

	/// Read the `start` field
	bool start() const SYMBOL_VISIBLE;

	/// Set the `start` field
	void start(bool value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x50c0;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  Write-only register file HicannBufferCounterReset.
 *  Writing a `1` will reset the start_address, size and threshold of TraceBufferCounter.
 *
 *  For a high-level interface use the configure_fpga method.
 */
struct HicannBufferCounterReset
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;

	/// Initialize all fields with zero
	HicannBufferCounterReset() = default;

	/// Initialize all fields with a specific value
	HicannBufferCounterReset(bool reset_) SYMBOL_VISIBLE;

	/// Read the `reset` field
	bool reset() const SYMBOL_VISIBLE;

	/// Set the `reset` field
	void reset(bool value) SYMBOL_VISIBLE;

	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x50a0;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = false;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/**
 *  The following code is only required until the trace buffer has been
 *  removed from the FPGA and should then be removed.
 */

/// Read-write register file TraceBufferStart.
/// Address in bytes of the start of the trace-pulse data ringbuffer.
///
/// This can be a physical or a logical address. For a logical address
/// the HostEndpoint node must be set to `0x4`.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferStart
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferStart() = default;
	/// Initialize the single field with a specific value
	TraceBufferStart(uint64_t value) : raw(value & 0xffffffffffffffff) {}
	/// Read the single field
	uint64_t data() const SYMBOL_VISIBLE;
	/// Set the single field
	void data(uint64_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5000;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-write register file TraceBufferSize.
/// The capacity of the trace-pulse data ringbuffer in bytes.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferSize
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferSize() = default;
	/// Initialize all fields with a specific value
	TraceBufferSize(uint32_t data_) SYMBOL_VISIBLE;
	/// Read the `data` field
	uint32_t data() const SYMBOL_VISIBLE;
	/// Set the `data` field
	void data(uint32_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5008;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-write register file TraceBufferFullThreshold.
/// The threshold that determines the full state of the trace-pulse data ringbuffer.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferFullThreshold
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferFullThreshold() = default;
	/// Initialize all fields with a specific value
	TraceBufferFullThreshold(uint32_t data_) SYMBOL_VISIBLE;
	/// Read the `data` field
	uint32_t data() const SYMBOL_VISIBLE;
	/// Set the `data` field
	void data(uint32_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5010;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-only register file TraceBufferCounter.
/// Various counters that report the number of successful initializations of the
/// trace-pulse data ringbuffer and the number of wrap arounds of the buffer.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferCounter
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferCounter() = default;
	/// Initialize all fields with a specific value
	TraceBufferCounter(
	    uint32_t start_address_, uint32_t size_, uint32_t threshold_, uint32_t wraps_)
	    SYMBOL_VISIBLE;
	/// Read the `start_address` field
	uint32_t start_address() const SYMBOL_VISIBLE;
	/// Read the `size` field
	uint32_t size() const SYMBOL_VISIBLE;
	/// Read the `threshold` field
	uint32_t threshold() const SYMBOL_VISIBLE;
	/// Read the `wraps` field
	uint32_t wraps() const SYMBOL_VISIBLE;
	/// Set the `start_address` field
	void start_address(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `size` field
	void size(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `threshold` field
	void threshold(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `wraps` field
	void wraps(uint32_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5018;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = false;
};

/// Write-only register file TraceBufferCounterReset.
/// Writing a `1` will reset the start_address, size and threshold of TraceBufferCounter.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferCounterReset
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferCounterReset() = default;
	/// Initialize all fields with a specific value
	TraceBufferCounterReset(bool reset_) SYMBOL_VISIBLE;
	/// Read the `reset` field
	bool reset() const SYMBOL_VISIBLE;
	/// Set the `reset` field
	void reset(bool value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5020;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = false;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-write register file TraceBufferInit.
/// Writing a `1` will reconfigure the trace-pulse data ringbuffer according to
/// the previously written config values.
///
/// For a high-level interface use the configure_fpga method.
struct TraceBufferInit
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceBufferInit() = default;
	/// Initialize all fields with a specific value
	TraceBufferInit(bool start_) SYMBOL_VISIBLE;
	/// Read the `start` field
	bool start() const SYMBOL_VISIBLE;
	/// Set the `start` field
	void start(bool value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x5040;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-write register file TraceNotificationBehaviour.
/// The notification behavior of the trace-pulse data ringbuffer.
///
/// Frequency is the number of packets after which the Fpga will send a payload
/// notification to the host. Timeout specifies the number of clock cycles after
/// which the Fpga will send a notification in case the number of packets is lower
/// than the frequency.
struct TraceNotificationBehaviour
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	TraceNotificationBehaviour() = default;
	/// Initialize all fields with a specific value
	TraceNotificationBehaviour(uint32_t timeout_, uint32_t frequency_) SYMBOL_VISIBLE;
	/// Read the `timeout` field
	uint32_t timeout() const SYMBOL_VISIBLE;
	/// Read the `frequency` field
	uint32_t frequency() const SYMBOL_VISIBLE;
	/// Set the `timeout` field
	void timeout(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `frequency` field
	void frequency(uint32_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x52a8;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

/// Read-write register file info.
/// The identifying ids of the FPGA node.
///
/// Guid is the gloabal unique identifyer of the network-node.
/// Ndid is the node-id of the FPGA. This is the only writable field.
/// Socketid is the number of the FPGA's board socket on the cube or wafer-module.
/// Edgeid is the number of the FPGA's edge-board on the cube or wafer-module.
/// All fields except the node-id are read-only.
struct Info
{
	/// The raw bits used to send and receive data to and from the hardware.
	/// This member may be accessed directly. The concrete bit-fields are always
	/// synchronized with this value.
	uint64_t raw = 0;
	/// Initialize all fields with zero
	Info() = default;
	/// Initialize all fields with a specific value
	Info(uint32_t guid_, uint16_t ndid_, uint8_t waferid_, uint8_t socketid_, uint8_t edgeid_)
	    SYMBOL_VISIBLE;
	/// Read the `guid` field
	uint32_t guid() const SYMBOL_VISIBLE;
	/// Read the `ndid` field
	uint16_t ndid() const SYMBOL_VISIBLE;
	/// Read the `waferid` field
	uint8_t waferid() const SYMBOL_VISIBLE;
	/// Read the `socketid_` field
	uint8_t socketid() const SYMBOL_VISIBLE;
	/// Read the `edgeid` field
	uint8_t edgeid() const SYMBOL_VISIBLE;
	/// Set the `guid` field
	void guid(uint32_t value) SYMBOL_VISIBLE;
	/// Set the `ndid` field
	void ndid(uint16_t value) SYMBOL_VISIBLE;
	/// Set the `waferid` field
	void waferid(uint8_t value) SYMBOL_VISIBLE;
	/// Set the `socketid` field
	void socketid(uint8_t value) SYMBOL_VISIBLE;
	/// Set the `edgeid` field
	void edgeid(uint8_t value) SYMBOL_VISIBLE;
	/// The hardware address of the register file on the remote Fpga
	constexpr static RMA2_NLA rf_address = 0x8008;
	/// Indicates whether this field can be read on the software side
	constexpr static bool readable = true;
	/// Indicates whether this field can be written on the software side
	constexpr static bool writable = true;
};

} // namespace nhtl_extoll
