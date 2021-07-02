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
#include "FastNoise.hpp"
#include "Registry.hpp"
#include "ServerWorld.hpp"
#include "TerrainBuilder.hpp"

void TerrainBuilder::addGenerationJob(ServerWorld &world, const gk::Vector3i &pos, Heightmap &heightmap) {
	ServerChunk &chunk = world.getOrCreateChunkAlone(pos.x, pos.y, pos.z);
	if (chunk.isInitialized()) return;

	const HeightmapChunk &heightmapChunk = heightmap.getOrCreateChunk(chunk.x(), chunk.y());

	auto future = thread::DefaultThreadPool::submitJob([this](ServerChunk &chunk, const HeightmapChunk &heightmap) {
		generateChunk(chunk, heightmap, m_biomeSampler);

		return std::ref(chunk);
	}, std::ref(chunk), std::cref(heightmapChunk));

	m_futures.emplace_back(std::move(future));
}

void TerrainBuilder::generateChunkNow(ServerWorld &world, ServerChunk &chunk) {
	// const HeightmapChunk &heightmapChunk = world.heightmap().getOrCreateChunk(chunk.x(), chunk.y());
	// generateChunk(chunk, heightmapChunk, m_biomeSampler);
	// world.createChunkNeighbours();
}

void TerrainBuilder::update(ServerWorld &world) {
	for (auto it = m_futures.begin() ; it != m_futures.end() ; ) {
		if (it->future().valid() && it->future().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			std::reference_wrapper<ServerChunk> chunk = it->get();

			world.createChunkNeighbours(chunk.get());

			chunk.get().setInitialized(true);
			chunk.get().lightmap().updateLights();
			chunk.get().setReadyToSend();

			world.addChunkToUpdate(&chunk.get());
			world.addChunkToProcess(&chunk.get());

			it = m_futures.erase(it);
		}
		else
			++it;
	}
}

void TerrainBuilder::generateChunk(ServerChunk &chunk, const HeightmapChunk &heightmap, TerrainBiomeSampler &biomeSampler) {
	for(s8 y = 0 ; y < CHUNK_DEPTH ; y++) {
		for(s8 x = 0 ; x < CHUNK_WIDTH ; x++) {
			u16 biomeIndex = biomeSampler.getBiomeIndexAt(x + chunk.x() * CHUNK_WIDTH, y + chunk.y() * CHUNK_DEPTH);
			const Biome &biome = Registry::getInstance().getBiome(biomeIndex);

			// Land height
			double h = heightmap.landHeightAt(x, y);

			// Land blocks
			for(s8 z = 0 ; z < CHUNK_HEIGHT ; z++) {
				// Are we above "ground" level?
				if(z + chunk.z() * CHUNK_HEIGHT > h) {
					// If we are not yet up to sea level, fill with water blocks
					if (z + chunk.z() * CHUNK_HEIGHT < SEALEVEL) {
						chunk.setDataRaw(x, y, z, biome.getLiquidBlockID());
					}
					// Otherwise we are in the air
					else if (chunk.getBlock(x, y, z) == 0 && z == CHUNK_HEIGHT - 1) {
						// Add sunlight at the top of the chunk if possible
						chunk.lightmap().addSunlight(x, y, z, 15, true);
					}
				}
				else {
					bool isGeneratingTopBlock = z + chunk.z() * CHUNK_HEIGHT == h && z + chunk.z() * CHUNK_HEIGHT > SEALEVEL - 1;
					if (isGeneratingTopBlock)
						chunk.setDataRaw(x, y, z, biome.getTopBlockID());
					else if (z + chunk.z() * CHUNK_HEIGHT <= SEALEVEL - 1 && h < SEALEVEL && z + chunk.z() * CHUNK_HEIGHT > h - 3)
						chunk.setDataRaw(x, y, z, biome.getBeachBlockID());
					else if (z + chunk.z() * CHUNK_HEIGHT > h - 3)
						chunk.setDataRaw(x, y, z, biome.getGroundBlockID());
					else
						chunk.setDataRaw(x, y, z, biome.getDeepBlockID());
				}
			}
		}
	}
}

