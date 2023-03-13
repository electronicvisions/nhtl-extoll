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

	int m_type = 0;

public:
	RMA2_Port get_port() const;
	RMA2_Handle get_handle() const;
	RMA2_VPID get_vpid() const;

	/// RMA2_Connection_Options option for RRA connection.
	static inline RMA2_Connection_Options const rra_connection =
	    RMA2_Connection_Options(uint32_t(RMA2_CONN_PHYSICAL) | uint32_t(RMA2_CONN_RRA));

	/// Open a single connection to the remote node.
	/// @throws ConnectionFailed if an error happens inside `librma2`
	explicit Connection(RMA2_Nodeid, bool rra);
	/// This class is moveable as the underlying registered memory
	/// region is stable address-wise
	Connection(Connection&&) = default;
	/// This class is not copyable
	Connection(Connection const&) = delete;
	/// This class is not copy-assignable
	Connection& operator=(Connection const&) = delete;
	/// Closes all `librma2` resources
	~Connection();
};

/**
 *  Encapsulates the various handles needed for the `librma2` to represent a connection.
 *  Extoll keeps track of all Connections internally.
 */
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
	/**
	 * Maximum RF address available. This is determined by the register file
	 * and should be adjusted if the register file is changed.
	 */
	constexpr static uint64_t max_address = 0x180d0;

	constexpr static RMA2_NLA hicann_address = 0x2a1bull << 48;
	constexpr static RMA2_NLA trace_address = 0x0ca5ull << 48;

	RMA2_Nodeid get_node() const;

	RMA2_Port get_rra_port() const;
	RMA2_Handle get_rra_handle() const;
	RMA2_VPID get_rra_vpid() const;

	RMA2_Port get_rma_port() const;
	RMA2_Handle get_rma_handle() const;
	RMA2_VPID get_rma_vpid() const;

	NotificationPoller poller;

	/// A buffer that acts as a response buffer for RRA traffic and send buffer
	/// for RMA traffic.
	PhysicalBuffer buffer;
	/// The HICANN ring buffer
	/// Currently not used but required for successful configuration
	RingBuffer hicann_ring_buffer;
	/// The trace data ring buffer
	/// Currently used for all incoming RMA traffic
	RingBuffer trace_ring_buffer;

	/// Opens a connection to a remote node.
	/// @throws ConnectionFailed if there is an error inside `librma2`
	explicit Endpoint(RMA2_Nodeid);
	/// This class is moveable as the underlying registered memory
	/// region is stable address-wise
	Endpoint(Endpoint&&) = default;
	/// This class is not copyable
	Endpoint(Endpoint const&) = delete;
	/// This class is not copy-assignable
	Endpoint& operator=(Endpoint const&) = delete;

	/// Attempt to read the FPGA identifier at 0x8000 via RRA
	/// Returns true if the FPGA answers within 1ms, false otherwise
	bool ping() const;

	/**
	 *  Read the value of a register file.
	 *
	 *  Only read-write or read-only register files can be used with this method.
	 *  @code
	 *  auto reset = rf.read<Reset>();
	 *  std::cout << reset.core() << std::endl;
	 *  @endcode
	 */
	template <typename RF>
	RF rra_read() const
	{
		static_assert(RF::rf_address >= 0, "register file address must be positive!");
		static_assert(RF::rf_address <= max_address, "register file address too large!");
		static_assert(RF::readable, "register file must be readable!");

		RF rf;
		rf.raw = rra_read(RF::rf_address);
		return rf;
	}

	/**
	 * Write the value of a register file.
	 *
	 *  Only read-write or write-only register files can be used with this method.
	 *  @code
	 *  rf.write<HicannNotificationBehaviour>({0x100, 0x100});
	 *  @endcode
	 */
	template <typename RF>
	void rra_write(RF const& rf)
	{
		static_assert(RF::rf_address >= 0, "register file address must be positive!");
		static_assert(RF::rf_address <= max_address, "register file address too large!");
		static_assert(RF::writable, "register file must be writable!");

		rra_write(RF::rf_address, rf.raw);
	}

	/**
	 *  A non-template version of the read method.
	 *
	 *  This method is untyped and neither checks whether the remote register file
	 *  is readable nor does it unpack the bytes into the matching RF fields.
	 *
	 * Reading non-readable locations returns the data of the last readable
	 * location accessed. In particular, it is possible for bitfields in
	 * otherwise readable registers to be non readable and return garbage data.
	 */
	uint64_t rra_read(RMA2_NLA) const;

	/**
	 *  A non-template version of the write method.
	 *
	 *  This method is untyped and neither checks whether the remote register file
	 *  is writable nor does it provide a way to pack the fields into a quad word.
	 */
	void rra_write(RMA2_NLA, uint64_t);

	/**
	 * Send data via the RMA connection.
	 */
	void rma_send(size_t quad_words);
};

}
