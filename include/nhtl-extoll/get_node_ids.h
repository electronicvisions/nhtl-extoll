#pragma once
#include "hate/visibility.h"
#include "rma2.h"
#include <cstdint>
#include <vector>

namespace nhtl_extoll {

/**
 * Get list of all Node IDs available in the network.
 * @return Vector of Node IDs as uint16_t (RMA2_Nodeid)
 */
std::vector<RMA2_Nodeid> SYMBOL_VISIBLE get_all_node_ids();

/**
 * Check whether the link of a given Node ID belongs to a running FPGA.
 * @param node_id Node ID to be checked.
 * @return True if link is up and node_id belongs to a known FPGA.
 */
bool SYMBOL_VISIBLE check_is_fpga(RMA2_Nodeid node_id);

/**
 * Get list of Node IDs of FPGAs available in the network.
 * @return Vector of FPGA Node IDs as uint16_t (RMA2_Nodeid)
 */
std::vector<RMA2_Nodeid> SYMBOL_VISIBLE get_fpga_node_ids();

/**
 * Get Node ID of FPGA available in the network.
 * @return FPGA Node ID as uint16_t (RMA2_Nodeid)
 */
RMA2_Nodeid SYMBOL_VISIBLE get_fpga_node_id();

} // namespace nhtl_extoll
