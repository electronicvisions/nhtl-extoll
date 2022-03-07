#include "nhtl-extoll/connection.h"

#include "rma2_ioctl.h"
#include "sys/ioctl.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include "nhtl-extoll/exception.h"
#include "nhtl-extoll/throw_on_error.h"

namespace nhtl_extoll {

Connection::~Connection()
{
	if (m_port == nullptr) {
		assert(!m_handle);
		return;
	}

	if (m_handle != nullptr) {
		size_t ignored_notifications = 0;
		RMA2_Notification* notification;
		RMA2_ERROR status = rma2_noti_probe(m_port, &notification);
		while (status == RMA2_SUCCESS) {
			std::cerr << "Notification type: " << rma2_noti_get_notification_type(notification)
			          << "\n";
			std::cerr << "Notification VPID: " << rma2_noti_get_remote_vpid(notification) << "\n";
			std::cerr << "Notification Node ID: " << rma2_noti_get_remote_nodeid(notification)
			          << "\n";
			rma2_noti_free(m_port, notification);
			++ignored_notifications;
			status = rma2_noti_probe(m_port, &notification);
		}
		if (status == RMA2_ERR_INV_PORT) {
			throw_on_error<ConnectionFailed>(status, "Invalid port while closing connection!");
		}
		if (ignored_notifications) {
			std::cerr << "Ignored Notifications: " << ignored_notifications << std::endl;
		}
		rma2_disconnect(m_port, m_handle);
	}

	rma2_close(m_port);
}


Connection::Connection(RMA2_Nodeid node, bool rra)
{
	m_type = (rra ? rra_connection : RMA2_CONN_PHYSICAL);
	RMA2_ERROR status = rma2_open(&m_port);
	throw_on_error<ConnectionFailed>(status, "Failed to open port!");
	m_vpid = rma2_get_vpid(m_port);
	status =
	    rma2_connect(m_port, node, m_vpid, (rra ? rra_connection : RMA2_CONN_PHYSICAL), &m_handle);
	throw_on_error<ConnectionFailed>(status, "Failed to connect!");
}

RMA2_Port Connection::get_port() const
{
	return m_port;
}

RMA2_Handle Connection::get_handle() const
{
	return m_handle;
}

RMA2_VPID Connection::get_vpid() const
{
	return m_vpid;
}


Endpoint::Endpoint(RMA2_Nodeid n) :
    m_node(n),
    m_rra(n, true),
    m_rma(n, false),
    poller(get_rma_port()),
    buffer(),
    hicann_ring_buffer(get_rma_port(), get_rma_handle(), poller, 1),
    trace_ring_buffer(get_rma_port(), get_rma_handle(), poller, 2048)
{}

RMA2_Nodeid Endpoint::get_node() const
{
	return m_node;
}

RMA2_Port Endpoint::get_rra_port() const
{
	return m_rra.get_port();
}

RMA2_Handle Endpoint::get_rra_handle() const
{
	return m_rra.get_handle();
}

RMA2_VPID Endpoint::get_rra_vpid() const
{
	return m_rra.get_vpid();
}

RMA2_Port Endpoint::get_rma_port() const
{
	return m_rma.get_port();
}

RMA2_Handle Endpoint::get_rma_handle() const
{
	return m_rma.get_handle();
}

RMA2_VPID Endpoint::get_rma_vpid() const
{
	return m_rma.get_vpid();
}

uint64_t Endpoint::rra_read(RMA2_NLA address) const
{
	RMA2_ERROR status = rma2_post_get_qw_direct(
	    get_rra_port(), get_rra_handle(), buffer.response_address(), 8, address,
	    RMA2_COMPLETER_NOTIFICATION, RMA2_CMD_DEFAULT);

	throw_on_error<FailedToRead>(status, get_node(), address);

	RMA2_Notification* notification;
	status = rma2_noti_get_block(get_rra_port(), &notification);
	throw_on_error<FailedToRead>(status, get_node(), address);
	status = rma2_noti_free(get_rra_port(), notification);
	throw_on_error<FailedToRead>(status, get_node(), address);

	return buffer.read_response();
}

void Endpoint::rra_write(RMA2_NLA address, uint64_t value)
{
	RMA2_ERROR status = rma2_post_immediate_put(
	    get_rra_port(), get_rra_handle(), 8, value, address, RMA2_COMPLETER_NOTIFICATION,
	    RMA2_CMD_DEFAULT);
	throw_on_error<FailedToWrite>(status, get_node(), address);

	RMA2_Notification* notification;
	status = rma2_noti_get_block(get_rra_port(), &notification);
	throw_on_error<FailedToWrite>(status, get_node(), address);
	status = rma2_noti_free(get_rra_port(), notification);
	throw_on_error<FailedToWrite>(status, get_node(), address);
}

void Endpoint::rma_send(size_t quad_words)
{
	RMA2_ERROR status = rma2_post_put_qw_direct(
	    get_rma_port(), get_rma_handle(), buffer.send_address(), sizeof(uint64_t) * quad_words,
	    trace_address, RMA2_NO_NOTIFICATION, RMA2_CMD_DEFAULT);
	throw_on_error<FailedToWrite>(status, get_node(), trace_address);
}

} // namespace nhtl_extoll
