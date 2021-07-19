/*
 * =====================================================================================
 *
 *  OpenMiner
 *
 *  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
 *  Copyright (C) 2019-2020 the OpenMiner contributors (see CONTRIBUTORS.md)
 *
 *  This file is part of OpenMiner.
 *
 *  OpenMiner is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  OpenMiner is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with OpenMiner; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * =====================================================================================
 */
#ifndef BLOCKPARAM_HPP_
#define BLOCKPARAM_HPP_

#include <map>
#include <string>

#include <gk/core/IntTypes.hpp>
#include <gk/core/ISerializable.hpp>

#include "NetworkUtils.hpp"

namespace sol { class state; }

class Block;

class BlockParam : public gk::ISerializable {
	public:
		BlockParam(Block &block) : m_block(&block) {}

		void serialize(sf::Packet &packet) const override;
		void deserialize(sf::Packet &packet) override;

		enum Type {
			Rotation    = 0,
			State       = 1,
			Custom      = 2,
			WallMounted = 3,

			Count
		};

		void allocateBits(u8 type, u8 size);
		u8 getAllocatedSize() const { return m_totalSize; }

		u16 getParam(u8 type, u16 data) const;
		u16 setParam(u8 type, u16 data, u16 param) const;
		bool hasParam(u8 type) const { return m_allocatedBits.find(type) != m_allocatedBits.end(); }

		static std::string getTypeName(u8 type);

		static void initUsertype(sol::state &lua);

	private:
		Block *m_block = nullptr;
		u8 m_totalSize = 0;

		struct Param : public gk::ISerializable {
			Param() = default;
			Param(u8 _offset, u8 _size) : offset(_offset), size(_size) {}

			void serialize(sf::Packet &packet) const override { packet << offset << size; }
			void deserialize(sf::Packet &packet) override { packet >> offset >> size; }

			u8 offset;
			u8 size;
		};

		std::map<u8, Param> m_allocatedBits;
};

#endif // BLOCKPARAM_HPP_
