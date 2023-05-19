#pragma once
#include "hate/visibility.h"
#include "rma2.h"
#include <stdexcept>

namespace nhtl_extoll {

/// The base class for all exceptions in this library.
/// Only its child classes will be instantiated.
struct RmaError : std::runtime_error
{
	using runtime_error::runtime_error;

protected:
	RmaError() SYMBOL_VISIBLE;
};

/// This exception indicates that a connection to a remote Fpga could not be established.
struct ConnectionFailed : RmaError
{
	using RmaError::RmaError;
};

/// This exception indicates that a user-space buffer could not be registered with the extoll
/// driver. This can occur if too many regions are already registered with the driver
struct FailedToRegisterRegion : RmaError
{
	/// Creates an exception
	FailedToRegisterRegion() SYMBOL_VISIBLE;
};

/// This exception indicates that a remote register file access has failed.
///
/// Only its child classes will be instantiated.
struct RraError : RmaError
{
protected:
	/// Creates an exception from a message, a node id and a network logical address
	RraError(std::string const& msg, RMA2_Nodeid n, RMA2_NLA a) SYMBOL_VISIBLE;

	/// The node id of the node that caused the error
	RMA2_Nodeid node;
	/// The register file address that was accessed
	RMA2_NLA address;
};

/// This exception indicates that a remote register file access read command failed
struct FailedToRead : RraError
{
	using RraError::RraError;
	/// Creates an exception from a node id and a network logical address
	FailedToRead(RMA2_Nodeid node, RMA2_NLA address) SYMBOL_VISIBLE;
};

/// This exception indicates that a remote register file access write command failed
struct FailedToWrite : RraError
{
	using RraError::RraError;
	/// Creates an exception from a node id and a network logical address
	FailedToWrite(RMA2_Nodeid node, RMA2_NLA address) SYMBOL_VISIBLE;
};

/// This exception occurs when the user tries to connect to a remote node that is not an
/// properly configured FPGA
struct NodeIsNoFpga : ConnectionFailed
{
	/// Creates an exception from a node id and an incorrect observed driver version
	NodeIsNoFpga(RMA2_Nodeid node, uint32_t driver) SYMBOL_VISIBLE;

	/// The node id of the node that couldn't be connected
	RMA2_Nodeid const node;
	/// The driver version of the remote node.
	///
	/// The value is `0xcafebabe` for properly configured FPGAs
	uint32_t const driver;
};

}
