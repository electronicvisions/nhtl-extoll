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


Connection::Connection(RMA2_Nodeid node)
{
	RMA2_ERROR status = rma2_open(&m_port);
	throw_on_error<ConnectionFailed>(status, "Failed to open port!");
	m_vpid = rma2_get_vpid(m_port);
	status = rma2_connect(m_port, node, m_vpid, rra_connection, &m_handle);
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


Endpoint::Endpoint(RMA2_Nodeid n) : m_node(n), m_rra(n), buffer(m_rra.get_port(), 1) {}

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

} // namespace nhtl_extoll
