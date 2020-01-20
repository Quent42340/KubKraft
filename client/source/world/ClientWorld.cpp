/*
 * =====================================================================================
 *
 *       Filename:  ClientWorld.cpp
 *
 *    Description:
 *
 *        Created:  12/01/2019 14:01:17
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include <glm/gtc/matrix_transform.hpp>

#include <gk/gl/Shader.hpp>
#include <gk/resource/ResourceHandler.hpp>

#include "ClientCommandHandler.hpp"
#include "ClientWorld.hpp"
#include "World.hpp"

ClientWorld::ClientWorld() :
	m_texture(gk::ResourceHandler::getInstance().get<gk::Texture>("texture-blocks"))
{
}

void ClientWorld::init(float playerX, float playerY, float playerZ) {
	int pcx = std::floor(playerX / CHUNK_WIDTH);
	int pcy = std::floor(playerY / CHUNK_HEIGHT);
	int pcz = std::floor(playerZ / CHUNK_DEPTH);

	m_chunks.emplace(gk::Vector3i{pcx, pcy, pcz}, new ClientChunk(pcx, pcy, pcz, m_texture));
}

void ClientWorld::update() {
	// Update loaded chunks
	for (auto &it : m_chunks) {
		if (World::isReloadRequested)
			it.second->setChanged(true);

		it.second->update();
	}

	World::isReloadRequested = false;

	sendChunkRequests();
}

void ClientWorld::sendChunkRequests() {
	// If we have a chunk marked for initialization
	if (m_ud < 1000) {
		ClientChunk *chunk = (ClientChunk *)getChunk(m_ux, m_uy, m_uz);
		if(chunk && !chunk->hasBeenRequested()) {
			// Send a chunk request to the server
			m_client->sendChunkRequest(m_ux, m_uy, m_uz);
			chunk->setHasBeenRequested(true);

			// std::cout << "Chunk at (" << m_ux << ", " << m_uy << ", " << m_uz << ") requested" << std::endl;
		}
	}
}

void ClientWorld::receiveChunkData(sf::Packet &packet) {
	s32 cx, cy, cz;
	packet >> cx >> cy >> cz;

	// Get the chunk from the map or create it if it doesn't exist
	ClientChunk *chunk = (ClientChunk *)getChunk(cx, cy, cz);
	if (!chunk) {
		auto it = m_chunks.emplace(gk::Vector3i{cx, cy, cz}, new ClientChunk(cx, cy, cz, m_texture));
		chunk = it.first->second.get();
	}

	createChunkNeighbours(chunk);

	// Receive chunk data
	for (u16 x = 0 ; x < CHUNK_WIDTH ; ++x) {
		for (u16 y = 0 ; y < CHUNK_HEIGHT ; ++y) {
			for (u16 z = 0 ; z < CHUNK_DEPTH ; ++z) {
				u16 block;
				u8 light;

				packet >> block >> light;

				chunk->setBlockRaw(x, y, z, block & 0xffff);
				// chunk->setData(x, y, z, block >> 16);
				chunk->lightmap().setLightData(x, y, z, light);
			}
		}
	}

	chunk->setInitialized(true);

	// std::cout << "Chunk at (" << cx << ", " << cy << ", " << cz << ") received" << std::endl;
}

Chunk *ClientWorld::getChunk(int cx, int cy, int cz) const {
	auto it = m_chunks.find({cx, cy, cz});
	if (it == m_chunks.end())
		return nullptr;

	return it->second.get();
}

void ClientWorld::createChunkNeighbours(ClientChunk *chunk) {
	gk::Vector3i surroundingChunks[6] = {
		{chunk->x() - 1, chunk->y(),     chunk->z()},
		{chunk->x() + 1, chunk->y(),     chunk->z()},
		{chunk->x(),     chunk->y(),     chunk->z() - 1},
		{chunk->x(),     chunk->y(),     chunk->z() + 1},
		{chunk->x(),     chunk->y() - 1, chunk->z()},
		{chunk->x(),     chunk->y() + 1, chunk->z()},
	};

	// Create entries in the map for surrounding chunks
	// That will allow draw() to mark them for initialization
	for (u8 i = 0 ; i < 6 ; ++i) {
		const s32 scx = surroundingChunks[i].x;
		const s32 scy = surroundingChunks[i].y;
		const s32 scz = surroundingChunks[i].z;

		ClientChunk *neighbour = (ClientChunk *)getChunk(scx, scy, scz);
		if (!neighbour) {
			auto it = m_chunks.emplace(gk::Vector3i{scx, scy, scz}, new ClientChunk(scx, scy, scz, m_texture));
			neighbour = it.first->second.get();
		}

		chunk->setSurroundingChunk(i, neighbour);
		neighbour->setSurroundingChunk((i % 2 == 0) ? i + 1 : i - 1, chunk);
	}
}

void ClientWorld::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	if (!target.getView()) {
		DEBUG("ERROR: Trying to draw world without a camera");
		return;
	}

	gk::Shader::bind(states.shader);
	states.shader->setUniform("u_renderDistance", Config::renderDistance * CHUNK_WIDTH);
	gk::Shader::bind(nullptr);

	m_ud = 1000.0;
	m_ux = 0;
	m_uy = 0;
	m_uz = 0;

	std::vector<std::pair<ClientChunk*, gk::Transform>> chunks;
	for(auto &it : m_chunks) {
		states.transform = glm::translate(glm::mat4(1.0f),
		                                  glm::vec3(it.second->x() * CHUNK_WIDTH,
		                                            it.second->y() * CHUNK_HEIGHT,
		                                            it.second->z() * CHUNK_DEPTH));

		// Is the chunk close enough?
		glm::vec4 center = target.getView()->getViewTransform().getMatrix()
		                 * states.transform.getMatrix()
		                 * glm::vec4(CHUNK_WIDTH / 2, CHUNK_HEIGHT / 2, CHUNK_DEPTH / 2, 1);

		// Nope, too far, don't render it
		if(glm::length(center) > (Config::renderDistance + 1) * CHUNK_WIDTH) {
			// FIXME: Remove chunks here
			continue;
		}

		// Is this chunk on the screen?
		center = target.getView()->getTransform().getMatrix() * center;

		float d = glm::length(center);
		center.x /= center.w;
		center.y /= center.w;

		// If it is behind the camera, don't bother drawing it
		if(center.z < -CHUNK_HEIGHT / 2) {
			continue;
		}

		// If it is outside the screen, don't bother drawing it
		if(fabsf(center.x) > 1 + fabsf(CHUNK_HEIGHT * 2 / center.w)
		|| fabsf(center.y) > 1 + fabsf(CHUNK_HEIGHT * 2 / center.w)) {
			continue;
		}

		// If this chunk is not initialized, skip it
		if(!it.second->isInitialized() && !it.second->hasBeenRequested()) {
			// But if it is the closest to the camera, mark it for initialization
			if(d < m_ud) {
				m_ud = d;
				m_ux = it.second->x();
				m_uy = it.second->y();
				m_uz = it.second->z();
			}

			continue;
		}

		// Only draw the chunk if all its neighbours are initialized
		chunks.emplace_back(it.second.get(), states.transform);
	}

	for (u8 i = 0 ; i < ChunkBuilder::layers ; ++i) {
		for (auto &it : chunks) {
			states.transform = it.second;
			it.first->drawLayer(target, states, i);
		}
	}
}

