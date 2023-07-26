#include <cstdint>
#include <vector>
#include <gtest/gtest.h>

#include "nhtl-extoll/configure_fpga.h"
#include "nhtl-extoll/connection.h"
#include "nhtl-extoll/get_node_ids.h"
#include "rma2.h"

TEST(DISABLED_TestExtollFPGA, CheckLinks)
{
	using namespace nhtl_extoll;
	typedef uint16_t link_id;
	std::map<RMA2_Nodeid, link_id> link_table{{1, 1}, {2, 5}, {4, 0}, {5, 3}};
	std::vector<RMA2_Nodeid> node_ids = get_all_node_ids();
	for (auto i : node_ids) {
		if (link_table.contains(i)) {
			EXPECT_TRUE(check_is_fpga(i));
		}
	}
}

TEST(DISABLED_TestExtollFPGA, CheckFPGA)
{
	using namespace nhtl_extoll;
	std::vector<RMA2_Nodeid> node_ids = get_fpga_node_ids();
	// Address and content of the register file identifying FPGAs.
	RMA2_NLA fpga_address = 0x8000;
	uint64_t fpga_identifier = 0xcafebabe;
	int fpga_count = 0;
	for (auto const& node_id : node_ids) {
		Endpoint connection{node_id};
		if (connection.rra_read(fpga_address) == fpga_identifier) {
			fpga_count++;
		}
	}
	ASSERT_GE(fpga_count, node_ids.size());
}

TEST(DISABLED_TestExtollFPGA, ConfigureFPGA)
{
	using namespace nhtl_extoll;
	std::vector<RMA2_Nodeid> node_ids = get_fpga_node_ids();
	// Address and content of the register file identifying FPGAs.
	RMA2_NLA fpga_address = 0x8000;
	uint64_t fpga_identifier = 0xcafebabe;
	int fpga_count = 0;
	for (auto const& node_id : node_ids) {
		try {
			Endpoint connection{node_id};
			if (connection.rra_read(fpga_address) == fpga_identifier) {
				fpga_count++;
				configure_fpga(connection);
				ASSERT_EQ(
				    connection.rra_read<TraceBufferStart>().data(),
				    connection.trace_ring_buffer.address(0));
			}
		} catch (const std::runtime_error& e) {
			std::cerr << e.what();
		}
	}
	ASSERT_GE(fpga_count, node_ids.size());
}
