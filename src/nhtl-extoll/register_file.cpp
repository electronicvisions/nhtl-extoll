#include "nhtl-extoll/register_file.h"

#include "nhtl-extoll/exception.h"
#include "nhtl-extoll/throw_on_error.h"

namespace nhtl_extoll {

RegisterFile::RegisterFile(Endpoint& connection) : m_connection(connection) {}

uint64_t RegisterFile::read(RMA2_NLA address) const
{
	RMA2_ERROR status = rma2_post_get_qw(
	    m_connection.get_rra_port(), m_connection.get_rra_handle(), m_connection.buffer.region(), 0,
	    8, address, RMA2_COMPLETER_NOTIFICATION, RMA2_CMD_DEFAULT);

	throw_on_error<FailedToRead>(status, m_connection.get_node(), address);

	RMA2_Notification* notification;
	status = rma2_noti_get_block(m_connection.get_rra_port(), &notification);
	throw_on_error<FailedToRead>(status, m_connection.get_node(), address);
	status = rma2_noti_free(m_connection.get_rra_port(), notification);
	throw_on_error<FailedToRead>(status, m_connection.get_node(), address);

	return m_connection.buffer[0];
}

void RegisterFile::write(RMA2_NLA address, uint64_t value)
{
	RMA2_ERROR status = rma2_post_immediate_put(
	    m_connection.get_rra_port(), m_connection.get_rra_handle(), 8, value, address,
	    RMA2_COMPLETER_NOTIFICATION, RMA2_CMD_DEFAULT);
	throw_on_error<FailedToWrite>(status, m_connection.get_node(), address);

	RMA2_Notification* notification;
	status = rma2_noti_get_block(m_connection.get_rra_port(), &notification);
	throw_on_error<FailedToWrite>(status, m_connection.get_node(), address);
	status = rma2_noti_free(m_connection.get_rra_port(), notification);
	throw_on_error<FailedToWrite>(status, m_connection.get_node(), address);
}

} // namespace nhtl_extoll
