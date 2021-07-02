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
#ifndef TERRAINBUILDER_HPP_
#define TERRAINBUILDER_HPP_

#include <thread/ThreadPool.hpp>

#include "Heightmap.hpp"
#include "ServerChunk.hpp"
#include "TerrainBiomeSampler.hpp"

class ServerWorld;

class TerrainBuilder {
	public:
		TerrainBuilder(const Dimension &dimension, s32 seed) : m_biomeSampler(dimension, seed) {}

		void addGenerationJob(ServerWorld &world, const gk::Vector3i &pos, Heightmap &heightmap);
		void generateChunkNow(ServerWorld &world, ServerChunk &chunk);

		void update(ServerWorld &world);

	private:
		static void generateChunk(ServerChunk &chunk, const HeightmapChunk &heightmap, TerrainBiomeSampler &biomeSampler);

		TerrainBiomeSampler m_biomeSampler;

		std::vector<thread::ThreadPool::TaskFuture<std::reference_wrapper<ServerChunk>>> m_futures;
};

#endif // TERRAINBUILDER_HPP_
