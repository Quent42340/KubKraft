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
#include <gk/core/GameClock.hpp>

#include "Dimension.hpp"
#include "EngineConfig.hpp"
#include "Network.hpp"
#include "PlayerList.hpp"
#include "Server.hpp"
#include "ServerCommandHandler.hpp"
#include "ServerConfig.hpp"
#include "ServerPlayer.hpp"
#include "ServerWorld.hpp"

ServerWorld::ServerWorld(PlayerList &players, const Dimension &dimension, s32 seed)
	: m_players(players),
	  m_dimension(dimension),
	  m_heightmap(seed),
	  m_terrainGenerator(m_heightmap, dimension, seed),
	  m_terrainBuilder(dimension, seed),
	  m_scene(players),
	  m_seed(seed)
{
}

void ServerWorld::update(bool doTick) {
	World::update();

	processSendRequests();

	if (doTick) {
		for (auto &it : m_chunks) // TODO: Loop on a subset of this hashmap
			it.second->tick();

		m_scene.update();
	}

	m_terrainBuilder.update(*this);
}

void ServerWorld::updatePlayerChunks(ServerPlayer &player, s32 cx, s32 cy, s32 cz) {
	gk::Vector3i currentChunk{cx, cy, cz};

	player.sentChunks.clear();
	m_chunkSendRequestQueue.emplace(currentChunk, player, gk::GameClock::getInstance().getTicks(true));

	std::list<gk::Vector3i> chunksToRemove;
	for (auto &chunkPos : player.loadedChunks()) {
		glm::dvec3 chunkWorldPos{
			chunkPos.x * CHUNK_WIDTH + CHUNK_WIDTH / 2,
			chunkPos.y * CHUNK_DEPTH + CHUNK_DEPTH / 2,
			chunkPos.z * CHUNK_HEIGHT + CHUNK_HEIGHT / 2
		};

		glm::dvec3 playerPos{
			player.x(),
			player.y(),
			player.z()
		};

		if (glm::length(playerPos - chunkWorldPos) >= (ServerConfig::renderDistance + 3) * CHUNK_WIDTH) {
			m_server->sendChunkUnload(chunkPos.x, chunkPos.y, chunkPos.z, player.client());
			chunksToRemove.emplace_back(chunkPos);

			// gkDebug() << "Chunk at" << chunkPos.x << chunkPos.y << chunkPos.z << "unloaded for player" << player.name();
		}
	}

	for (auto &chunkPos : chunksToRemove)
		player.removeLoadedChunk(chunkPos);
}

void ServerWorld::createChunkNeighbours(ServerChunk &chunk) {
	gk::Vector3i surroundingChunks[6] = {
		{chunk.x() - 1, chunk.y(),     chunk.z()},
		{chunk.x() + 1, chunk.y(),     chunk.z()},
		{chunk.x(),     chunk.y() - 1, chunk.z()},
		{chunk.x(),     chunk.y() + 1, chunk.z()},
		{chunk.x(),     chunk.y(),     chunk.z() - 1},
		{chunk.x(),     chunk.y(),     chunk.z() + 1},
	};

	for (u8 i = 0 ; i < 6 ; ++i) {
		// Check if this neighbour already exists, if yes then skip it
		ServerChunk *neighbour = (ServerChunk *)getChunk(surroundingChunks[i].x, surroundingChunks[i].y, surroundingChunks[i].z);
		if (neighbour) {
			// Assign surrounding chunk pointers
			chunk.setSurroundingChunk(i, neighbour);
			neighbour->setSurroundingChunk((i % 2 == 0) ? i + 1 : i - 1, &chunk);

			continue;
		}

		// Create our neighbour
		auto it = m_chunks.emplace(
			gk::Vector3i{
				surroundingChunks[i].x,
				surroundingChunks[i].y,
				surroundingChunks[i].z
			},
			new ServerChunk{
				surroundingChunks[i].x,
				surroundingChunks[i].y,
				surroundingChunks[i].z,
				*this
			}
		);

		// Get the created neighbour
		neighbour = it.first->second.get();

		// Assign surrounding chunk pointers
		chunk.setSurroundingChunk(i, neighbour);
		neighbour->setSurroundingChunk((i % 2 == 0) ? i + 1 : i - 1, &chunk);
	}
}

ServerChunk &ServerWorld::getOrCreateChunk(s32 cx, s32 cy, s32 cz) {
	ServerChunk &chunk = getOrCreateChunkAlone(cx, cy, cz);

	createChunkNeighbours(chunk);

	return chunk;
}

ServerChunk &ServerWorld::getOrCreateChunkAlone(s32 cx, s32 cy, s32 cz) {
	ServerChunk *chunk = (ServerChunk *)getChunk(cx, cy, cz);
	if (!chunk) {
		auto it = m_chunks.emplace(gk::Vector3i{cx, cy, cz}, new ServerChunk(cx, cy, cz, *this));
		chunk = it.first->second.get();
	}

	return *chunk;
}

Chunk *ServerWorld::getChunk(int cx, int cy, int cz) const {
	auto it = m_chunks.find({cx, cy, cz});
	if (it == m_chunks.end())
		return nullptr;

	return it->second.get();
}

void ServerWorld::processSendRequests() {
	// Debug variables
	// u64 chunksGenerated = 0;
	// u64 chunksSent = 0;
	// u64 chunksTooOld = 0;

	// if (!m_chunkSendRequestQueue.empty())
	// 	gkDebug() << "Processing send requests...";

	u64 start = gk::GameClock::getInstance().getTicks(true);
	u64 now = start;
	while (now - start < 500 && !m_chunkSendRequestQueue.empty()) {
		auto &[chunkPos, player, timestamp] = m_chunkSendRequestQueue.front();

		bool isTooOld = now - timestamp >= 10000;
		if (!isTooOld && player.sentChunks.find(chunkPos) == player.sentChunks.end()) {
			glm::dvec3 chunkWorldPos{
				chunkPos.x * CHUNK_WIDTH + CHUNK_WIDTH / 2,
				chunkPos.y * CHUNK_DEPTH + CHUNK_DEPTH / 2,
				chunkPos.z * CHUNK_HEIGHT + CHUNK_HEIGHT / 2
			};

			glm::dvec3 playerPos{
				player.x(),
				player.y(),
				player.z()
			};

			// bool addChunkBackToQueue = false;
			if (glm::length(playerPos - chunkWorldPos) < (ServerConfig::renderDistance + 2) * CHUNK_WIDTH) {
				if (player.isOnline() && !player.isChunkLoaded(chunkPos)) {
					// ServerChunk &chunk = getOrCreateChunk(chunkPos.x, chunkPos.y, chunkPos.z);

					// chunksGenerated +=
					m_terrainBuilder.addGenerationJob(*this, chunkPos, m_heightmap);

					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::West));
					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::East));
					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::South));
					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::North));
					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::Bottom));
					// generateChunk((ServerChunk &)*chunk.getSurroundingChunk(Chunk::Top));

					// if (!chunk.areAllNeighboursInitialized())
					// 	gkWarning() << "All neighbours of chunk" << chunkPos << "aren't initialized when they should be";
					// else {
					// 	chunk.lightmap().updateLights();
					// 	chunk.setReadyToSend();
                    //
					// 	addChunkToProcess(&chunk);
						player.addLoadedChunk(chunkPos);
                    //
					// 	// ++chunksSent;
					// }
				}

				// gkDebug() << "OK for chunk" << chunkPos.x << chunkPos.y << chunkPos.z << ":" << glm::length(playerPos - chunkWorldPos) << "<" << (int)ServerConfig::renderDistance * CHUNK_WIDTH;

				auto addChunkToQueue = [this](gk::Vector3i pos, s8 dx, s8 dy, s8 dz, ServerPlayer &player) {
					pos.x += dx; pos.y += dy; pos.z += dz;
					m_chunkSendRequestQueue.emplace(pos, player, gk::GameClock::getInstance().getTicks(true));
				};

				addChunkToQueue(chunkPos,  1,  0,  0, player);
				addChunkToQueue(chunkPos, -1,  0,  0, player);
				addChunkToQueue(chunkPos,  0,  1,  0, player);
				addChunkToQueue(chunkPos,  0, -1,  0, player);
				addChunkToQueue(chunkPos,  0,  0,  1, player);
				addChunkToQueue(chunkPos,  0,  0, -1, player);
			}

			// if (addChunkBackToQueue)
			// 	m_chunkSendRequestQueue.emplace(chunkPos, player, timestamp);
			// else
				player.sentChunks.emplace(chunkPos);
		}

		// if (isTooOld)
		// 	++chunksTooOld;

		m_chunkSendRequestQueue.pop();

		now = gk::GameClock::getInstance().getTicks(true);
	}

	// if (now - start > 0) {
	// 	gkDebug() << "Done in" << now - start << "ms for dim" << m_dimension.id()
	// 		<< "| Gen:" << chunksGenerated
	// 		<< "| Sent:" << chunksSent
	// 		// << "| BTQ:" << chunksBackToQueue
	// 		<< "| Old:" << chunksTooOld;
	// 		// << "| QLen:" << startQueueSize << "->" << m_chunkSendRequestQueue.size();
	// }
}

// Please update 'docs/lua-api-cpp.md' if you change this
void ServerWorld::initUsertype(sol::state &lua) {
	lua.new_usertype<ServerWorld>("ServerWorld",
		sol::base_classes, sol::bases<World>(),

		"dimension", &ServerWorld::dimension
	);
}

