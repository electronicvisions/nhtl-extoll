#include "nhtl-extoll/exception.h"

namespace nhtl_extoll {

NodeIsNoFpga::NodeIsNoFpga(RMA2_Nodeid n, uint32_t d) :
    ConnectionFailed("Connection to Fpga failed, because node is not an Fpga!"), node(n), driver(d)
{}

RraError::RraError(std::string const& msg, RMA2_Nodeid n, RMA2_NLA a) :
    RmaError(msg), node(n), address(a)
{}

FailedToRead::FailedToRead(RMA2_Nodeid n, RMA2_NLA a) : RraError("Failed to read!", n, a) {}

FailedToWrite::FailedToWrite(RMA2_Nodeid n, RMA2_NLA a) : RraError("Failed to write!", n, a) {}

FailedToRegisterRegion::FailedToRegisterRegion() : RmaError("Failed to register region") {}

} // namespace nhtl_extoll
