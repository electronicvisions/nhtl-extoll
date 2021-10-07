#include <cstdint>
#include <string>
#include <vector>
#include <boost/process.hpp>
#include <gtest/gtest.h>

#include "nhtl-extoll/configure_fpga.h"
#include "nhtl-extoll/connection.h"
#include "rma2.h"

std::vector<RMA2_Nodeid> get_node_ids()
{
	using namespace boost::process;
	std::vector<RMA2_Nodeid> nodes;
	std::string line;
	std::string output;
	ipstream pipe_stream;

	try {
		child c("emp-ctrl network listnodes", std_out > pipe_stream);

		while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
			output += line;
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		std::cerr << "Ensure module extoll is loaded." << '\n';
	}


	std::size_t pos = output.find_first_of('[');

	while (pos != std::string::npos) {
		auto sep = output.find_first_of('|', pos);
		if (sep == std::string::npos) {
			return nodes;
		}
		RMA2_Nodeid node_id(std::stoi(output.substr(pos + 1, sep - pos - 1)));
		nodes.push_back(node_id);

		pos = output.find_first_of('[', sep);
	}

	return nodes;
}

TEST(TestExtollFPGA, CheckFPGA)
{
	std::vector<RMA2_Nodeid> node_ids = get_node_ids();
	// Address and content of the register file identifying FPGAs.
	RMA2_NLA fpga_address = 0x8000;
	uint64_t fpga_identifier = 0xcafebabe;
	int fpga_count = 0;
	for (auto const& node_id : node_ids) {
		nhtl_extoll::Endpoint connection{node_id};
		if (connection.rra_read(fpga_address) == fpga_identifier) {
			fpga_count++;
		}
	}
	ASSERT_GE(fpga_count, 1);
}

TEST(TestExtollFPGA, ConfigureFPGA)
{
	std::vector<RMA2_Nodeid> node_ids = get_node_ids();
	// Address and content of the register file identifying FPGAs.
	RMA2_NLA fpga_address = 0x8000;
	uint64_t fpga_identifier = 0xcafebabe;
	int fpga_count = 0;
	for (auto const& node_id : node_ids) {
		nhtl_extoll::Endpoint connection{node_id};
		if (connection.rra_read(fpga_address) == fpga_identifier) {
			fpga_count++;
			nhtl_extoll::configure_fpga(connection);
			ASSERT_EQ(
			    connection.rra_read<nhtl_extoll::HicannBufferStart>().data(),
			    connection.ring_buffer.address(0));
		}
	}
	ASSERT_GE(fpga_count, 1);
}
