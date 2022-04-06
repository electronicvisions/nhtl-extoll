#include "nhtl-extoll/get_node_ids.h"

#include <iostream>
#include <string>
#include <boost/process.hpp>

namespace nhtl_extoll {

std::vector<RMA2_Nodeid> get_all_node_ids()
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

bool check_is_fpga(RMA2_Nodeid node_id)
{
	using namespace boost::process;
	std::string line;
	ipstream pipe_stream;

	typedef uint16_t link_id;
	std::map<RMA2_Nodeid, link_id> link_table{{1, 1}, {2, 5}, {4, 0}, {5, 3}};

	if (!link_table.contains(node_id)) {
		return false;
	}

	std::string path = getenv("EXTOLL_R2_SYSFS");
	path += "/extoll_rf_nw_lp_top_rf_lp" + std::to_string(link_table[node_id]) + "_status";
	std::string command = "grep -h ready " + path;

	try {
		child c(command, std_out > pipe_stream);

		while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
			if (line.end()[-2] == '1') {
				return true;
			}
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		std::cerr << "Ensure module extoll is loaded." << '\n';
	}

	return false;
}

std::vector<RMA2_Nodeid> get_fpga_node_ids()
{
	std::vector<RMA2_Nodeid> node_ids = get_all_node_ids();
	std::erase_if(node_ids, [](RMA2_Nodeid id) { return !check_is_fpga(id); });
	return node_ids;
}

RMA2_Nodeid get_fpga_node_id()
{
	auto const node_id_list = get_fpga_node_ids();
	if (node_id_list.empty()) {
		throw std::runtime_error("Extoll Error: No FPGA node found in environment.");
	}
	return node_id_list.at(0);
}

} // namespace nhtl_extoll
