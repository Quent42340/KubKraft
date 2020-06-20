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
#ifndef SERVERCHUNK_HPP_
#define SERVERCHUNK_HPP_

#include <atomic>

#include <gk/core/IntTypes.hpp>

#include "Chunk.hpp"

class ServerCommandHandler;
class ServerPlayer;

class ServerChunk : public Chunk {
	public:
		ServerChunk(s32 x, s32 y, s32 z, World &world) : Chunk(x, y, z, world) {}

		void updateLights();

		void onBlockPlaced(int x, int y, int z, const Block &block) override;
		void onBlockDestroyed(int x, int y, int z, const Block &block) override;

		void tick(World &world, ServerCommandHandler &server);

		bool isSent() const { return m_isSent; }
		void setSent(bool isSent) { m_isSent = isSent; }

		bool hasBeenModified() const { return m_hasBeenModified; }
		void setModified(bool hasBeenModified) { m_hasBeenModified = hasBeenModified; }

	private:
		std::atomic_bool m_isSent{false};
		std::atomic_bool m_hasBeenModified{false};
};

#endif // SERVERCHUNK_HPP_
