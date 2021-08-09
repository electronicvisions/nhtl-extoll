#pragma once

#include "nhtl-extoll/buffer.h"
#include "nhtl-extoll/notification_poller.h"
#include "rma2.h"

namespace nhtl_extoll {

/// A single connection to a remote node
struct Connection
{
private:
	/// The network node
	RMA2_Port m_port = nullptr;
	/// The handle
	RMA2_Handle m_handle = nullptr;
	/// The virtual process id
	RMA2_VPID m_vpid = RMA2_VPID_ANY;

public:
	RMA2_Port get_port() const;
	RMA2_Handle get_handle() const;
	RMA2_VPID get_vpid() const;

	/// RMA2_Connection_Options option for RRA connection.
	static inline RMA2_Connection_Options const rra_connection =
	    RMA2_Connection_Options(uint32_t(RMA2_CONN_DEFAULT) | uint32_t(RMA2_CONN_RRA));

	/// Open a single connection to the remote node.
	/// @throws ConnectionFailed if an error happens inside `librma2`
	explicit Connection(RMA2_Nodeid, bool rra);
	/// This class is moveable as the underlying registered memory region is stable address-wise
	Connection(Connection&&) = default;
	/// This class is not copyable
	Connection(Connection const&) = delete;
	/// This class is not copy-assignable
	Connection& operator=(Connection const&) = delete;
	/// Closes all `librma2` resources
	~Connection();
};

/// Encapsulates the various handles needed for the `librma2` to represent a connection.
///
/// Extoll keeps track of all Connections internally.
struct Endpoint
{
private:
	/// The node id of the remote node
	RMA2_Nodeid m_node;
	/// A remote register file connection
	Connection m_rra;
	/// A remote memory access connection
	Connection m_rma;

public:
	RMA2_Nodeid get_node() const;

	RMA2_Port get_rra_port() const;
	RMA2_Handle get_rra_handle() const;
	RMA2_VPID get_rra_vpid() const;

	RMA2_Port get_rma_port() const;
	RMA2_Handle get_rma_handle() const;
	RMA2_VPID get_rma_vpid() const;

	NotificationPoller poller;

	/// The buffer for all rra responses
	PhysicalBuffer buffer;
	/// The ring buffer for rma traffic
	RingBuffer ring_buffer;

	/// A mostly superfluous ring buffer required for successful configuration
	/// Remove when trace ring buffer is removed from FPGA
	RingBuffer trace_ring_buffer;

	/// Opens a connection to a remote node.
	/// @throws ConnectionFailed if there is an error inside `librma2`
	/// @throws FailedToWrite if the write test failed
	/// @throws FailedToRead if the read test failed
	explicit Endpoint(RMA2_Nodeid);
	/// This class is moveable as the underlying registered memory region is stable address-wise
	Endpoint(Endpoint&&) = default;
	/// This class is not copyable
	Endpoint(Endpoint const&) = delete;
	/// This class is not copy-assignable
	Endpoint& operator=(Endpoint const&) = delete;
};

}