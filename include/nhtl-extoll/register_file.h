#pragma once

#include <utility>

#include "nhtl-extoll/connection.h"
#include "rma2.h"

namespace nhtl_extoll {

/**
 *  The remote register file access interface.
 *  This class should be instantiated through the factory functions in Extoll
 */
class RegisterFile
{
protected:
	/**
	 *  A reference to the remote connection.
	 *  Because of this reference, the class depends on the Extoll object that
	 *  was used to create the register file interface.
	 */
	Endpoint& m_connection;

	/**
	 * Maximum RF address available. This is determined by the register file
	 * and should be adjusted if the register file is changed.
	 */

	constexpr static int max_address = 0x180d0;

public:
	/**
	 *  Create a register file interface from a given Endpoint.
	 *  This constructor should not be used directly. Use the factory functions
	 *  in Extoll instead.
	 */
	explicit RegisterFile(Endpoint&);
	/// This class is not copyable
	RegisterFile(RegisterFile const&) = delete;
	/// This class is movable
	RegisterFile(RegisterFile&&) noexcept = default;
	/// This class is not copy-assignable
	RegisterFile& operator=(RegisterFile const&) = delete;

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
	RF read() const
	{
		static_assert(RF::address >= 0, "register file address must be positive!");
		static_assert(RF::address <= max_address, "register file address too large!");
		static_assert(RF::readable, "register file must be readable!");

		RF rf;
		rf.raw = read(RF::address);
		return rf;
	}

	/**
	 * Write the value of a register file.
	 *
	 *  Only read-write or write-only register files can be used with this method.
	 *  @code
	 *  rf.write<JtagCmd>({JtagCmd::IR, 6, false, true});
	 *  @endcode
	 */
	template <typename RF>
	void write(RF&& rf)
	{
		static_assert(RF::address >= 0, "register file address must be positive!");
		static_assert(RF::address <= max_address, "register file address too large!");
		static_assert(RF::writable, "register file must be writable!");

		write(RF::address, rf.raw);
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
	uint64_t read(RMA2_NLA) const;

	/**
	 *  A non-template version of the write method.
	 *
	 *  This method is untyped and neither checks whether the remote register file
	 *  is writable nor does it provide a way to pack the fields into a quad word.
	 */
	void write(RMA2_NLA, uint64_t);
};

} // namespace nhtl_extoll
