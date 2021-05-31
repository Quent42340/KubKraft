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
#include "BlockGeometry.hpp"
#include "ChunkMeshBuilder.hpp"
#include "ClientWorld.hpp"
#include "TextureAtlas.hpp"

using namespace BlockGeometry;

void ChunkMeshBuilder::addMeshBuildingJob(const Chunk &chunk, const TextureAtlas &textureAtlas) {
	ChunkMeshBuildingJob job;
	job.textureAtlas = &textureAtlas;

	job.chunkData.x = chunk.x();
	job.chunkData.y = chunk.y();
	job.chunkData.z = chunk.z();

	std::memcpy(&job.chunkData.data, &chunk.data(), sizeof(Chunk::DataArray));
	std::memcpy(&job.chunkData.lightData, &chunk.lightmap().data(), sizeof(ChunkLightmap::LightMapArray));

	auto future = m_threadPool.submit([](ChunkMeshBuildingJob job) {
		for (s8f z = 0 ; z < CHUNK_HEIGHT ; z++) {
			for (s8f y = 0 ; y < CHUNK_DEPTH ; y++) {
				for (s8f x = 0 ; x < CHUNK_WIDTH ; x++) {
					u16 blockID = job.chunkData.getBlockID(x, y, z);
					if (!blockID) continue;

					u16 blockParam = job.chunkData.getBlockParam(x, y, z);
					const BlockState &blockState = job.chunkData.getBlockState(blockID, blockParam);

					if (blockState.drawType() == BlockDrawType::XShape)
						addCross(x, y, z, job, blockState);
					else
						addCube(x, y, z, job, blockState, blockParam);
				}
			}
		}

		return job;
	}, job);

	m_futures.emplace_back(std::move(future));
}

void ChunkMeshBuilder::update() {
	for (auto it = m_futures.begin() ; it != m_futures.end() ; ) {
		if (it->future().valid() && it->future().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			ChunkMeshBuildingJob job = it->get();

			ClientChunk *chunk = (ClientChunk *)m_world.getChunk(job.chunkData.x, job.chunkData.y, job.chunkData.z);
			if (chunk) {
				for (u8 i = 0 ; i < ChunkBuilder::layers ; ++i) {
					job.vertices[i].shrink_to_fit();

					const gk::VertexBuffer &vbo = chunk->getVertexBuffer(i);

					gk::VertexBuffer::bind(&vbo);
					vbo.setData(job.vertices[i].size() * sizeof(Vertex), job.vertices[i].data(), GL_DYNAMIC_DRAW);
					gk::VertexBuffer::bind(nullptr);

					chunk->setVerticesCount(i, job.vertices[i].size());
				}
			}

			it = m_futures.erase(it);
		}
		else
			++it;
	}
}

inline void ChunkMeshBuilder::addCube(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job, const BlockState &blockState, u16 blockParam) {
	const gk::FloatBox &boundingBox = blockState.boundingBox();

	u8f orientation = blockState.block().isRotatable()
		? blockState.block().param().getParam(BlockParam::Rotation, blockParam) : 0;
	const glm::mat3 &orientMatrix = orientMatrices[orientation];

	glm::vec3 vertexPos[nVertsPerCube]{
		// Order is important. It matches the bit order defined in BlockGeometry::cubeVerts.
		{boundingBox.x,                     boundingBox.y,                     boundingBox.z},
		{boundingBox.x + boundingBox.sizeX, boundingBox.y,                     boundingBox.z},
		{boundingBox.x,                     boundingBox.y + boundingBox.sizeY, boundingBox.z},
		{boundingBox.x + boundingBox.sizeX, boundingBox.y + boundingBox.sizeY, boundingBox.z},
		{boundingBox.x,                     boundingBox.y,                     boundingBox.z + boundingBox.sizeZ},
		{boundingBox.x + boundingBox.sizeX, boundingBox.y,                     boundingBox.z + boundingBox.sizeZ},
		{boundingBox.x,                     boundingBox.y + boundingBox.sizeY, boundingBox.z + boundingBox.sizeZ},
		{boundingBox.x + boundingBox.sizeX, boundingBox.y + boundingBox.sizeY, boundingBox.z + boundingBox.sizeZ},
	};

	if (blockState.drawType() == BlockDrawType::Cactus) {
		// Ignore bounding box, initialize it to full node coordinates
		for (u8f i = 0; i < nVertsPerCube; ++i) {
			vertexPos[i].x = (i >> 0) & 1;
			vertexPos[i].y = (i >> 1) & 1;
			vertexPos[i].z = (i >> 2) & 1;
		}
	}

	// vNeighbour is used to find neighbouring cubes per vertex.
	// Same binary layout.
	glm::vec3 vNeighbour[nVertsPerCube] = {
		{-1,-1,-1}, { 1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, {-1,-1, 1}, { 1,-1, 1}, {-1, 1, 1}, {1, 1, 1},
	};

	if (orientation) { // don't work extra if it's not oriented differently
		static const glm::vec3 half{0.5, 0.5, 0.5};
		// Rotate each vertex coordinate around the centre of the
		// cube, and each vertex neighbour around the origin
		for (int i = 0; i < nVertsPerCube; ++i) {
			vertexPos[i] = orientMatrix * (vertexPos[i] - half) + half;
			vNeighbour[i] = orientMatrix * vNeighbour[i];
		}
	}

	for (s8f f = 0; f < nFaces ; ++f) {
		// Construct the normal vector to a face
		const glm::vec3 glmNormal = orientMatrix * faceNormals[f];
		const gk::Vector3i normal{int(glmNormal.x), int(glmNormal.y), int(glmNormal.z)};

		// Construct an array with the 4 vertex positions of this face
		glm::vec3 *faceVerts[nVertsPerFace]{
			&vertexPos[cubeVerts[f][0]], &vertexPos[cubeVerts[f][1]],
			&vertexPos[cubeVerts[f][2]], &vertexPos[cubeVerts[f][3]]
		};

		// Construct an array with the 4 vertex neighbours of this face
		// (as GameKit integer vectors)
		const gk::Vector3i corner0{int(vNeighbour[cubeVerts[f][0]].x), int(vNeighbour[cubeVerts[f][0]].y), int(vNeighbour[cubeVerts[f][0]].z)};
		const gk::Vector3i corner1{int(vNeighbour[cubeVerts[f][1]].x), int(vNeighbour[cubeVerts[f][1]].y), int(vNeighbour[cubeVerts[f][1]].z)};
		const gk::Vector3i corner2{int(vNeighbour[cubeVerts[f][2]].x), int(vNeighbour[cubeVerts[f][2]].y), int(vNeighbour[cubeVerts[f][2]].z)};
		const gk::Vector3i corner3{int(vNeighbour[cubeVerts[f][3]].x), int(vNeighbour[cubeVerts[f][3]].y), int(vNeighbour[cubeVerts[f][3]].z)};

		const gk::Vector3i *vFaceNeighbours[nVertsPerFace]{&corner0, &corner1, &corner2, &corner3};

		addCubeFace(x, y, z, f, job, blockState, normal, faceVerts, vFaceNeighbours);
	}
}

inline void ChunkMeshBuilder::addCubeFace(s8f x, s8f y, s8f z, s8f f, ChunkMeshBuildingJob &job,
                                          const BlockState &blockState,
                                          const gk::Vector3i &normal,
                                          const glm::vec3 *const vertexPos[nVertsPerFace],
                                          const gk::Vector3i *const neighbourOfs[nVertsPerFace])
{
	// Get surrounding block for the face
	s8f sx = x + normal.x;
	s8f sy = y + normal.y;
	s8f sz = z + normal.z;

	const BlockState *surroundingBlockState = job.chunkData.getBlockState(sx, sy, sz);

	// Skip hidden faces
	if (surroundingBlockState && surroundingBlockState->block().id()
	&& ((blockState.drawType() == BlockDrawType::Solid && surroundingBlockState->drawType() == BlockDrawType::Solid && surroundingBlockState->isOpaque())
	 || (blockState.block().id() == surroundingBlockState->block().id() && (blockState.drawType() == BlockDrawType::Liquid || blockState.drawType() == BlockDrawType::Glass))
	 || (blockState.drawType() == BlockDrawType::Liquid && surroundingBlockState->drawType() == BlockDrawType::Solid)
	 || (blockState.drawType() == BlockDrawType::Cactus && surroundingBlockState->block().id() == blockState.block().id() && f > 3)))
		return;

	const gk::FloatBox &boundingBox = blockState.boundingBox();

	const std::string &texture = blockState.tiles().getTextureForFace(f);
	const gk::FloatRect &blockTexCoords = job.textureAtlas->getTexCoords(texture);

	// Calculate UV's
	// These are tough to obtain. Note that texture Y grows in the up-down direction, and so does V.
	// Vertex index in the bitmap array and U/V correspondence is:
	//    U0V0 -> 3 2 <- U1V0
	//    U0V1 -> 0 1 <- U1V1
	float U0, V0, U1, V1;
	if (blockState.drawType() == BlockDrawType::Cactus) {
		U0 = 0.f;
		V0 = 0.f;
		U1 = 1.f;
		V1 = 1.f;
	}
	else {
		U0 = (f == 0) ? 1.f - (boundingBox.y + boundingBox.sizeY) : (f == 1) ? boundingBox.y :
		     (f == 3) ? 1.f - (boundingBox.x + boundingBox.sizeX) : boundingBox.x;
		V0 = (f <= 3) ? 1.f - (boundingBox.z + boundingBox.sizeZ) : (f == 4) ? boundingBox.y : 1.f - (boundingBox.y + boundingBox.sizeY);
		U1 = (f == 0) ? 1.f - boundingBox.y : (f == 1) ? boundingBox.y + boundingBox.sizeY :
		     (f == 3) ? 1.f - boundingBox.x : boundingBox.x + boundingBox.sizeX;
		V1 = (f <= 3) ? 1.f - boundingBox.z : (f == 4) ? boundingBox.y + boundingBox.sizeY : 1.f - boundingBox.y;
	}

	// Prepare vertex information for VBO
	Vertex vertices[nVertsPerFace];
	for (s8f v = 0; v < nVertsPerFace; ++v) {
		if (blockState.drawType() == BlockDrawType::Cactus) {
			vertices[v].coord3d[0] = x + vertexPos[v]->x - boundingBox.x * normal.x;
			vertices[v].coord3d[1] = y + vertexPos[v]->y - boundingBox.y * normal.y;
			vertices[v].coord3d[2] = z + vertexPos[v]->z - boundingBox.z * normal.z;
		}
		else {
			float blockHeight = vertexPos[v]->z;
			if (blockState.drawType() == BlockDrawType::Liquid && (f != BlockFace::Bottom || !surroundingBlockState || !surroundingBlockState->block().id())) {
				if (f == BlockFace::Bottom)
					blockHeight = vertexPos[v]->z - 2.f / 16.f;
				else
					blockHeight = vertexPos[v]->z * 14.f / 16.f;
			}

			vertices[v].coord3d[0] = x + vertexPos[v]->x;
			vertices[v].coord3d[1] = y + vertexPos[v]->y;
			vertices[v].coord3d[2] = z + blockHeight;
		}

		vertices[v].coord3d[0] += blockState.drawOffset().x;
		vertices[v].coord3d[1] += blockState.drawOffset().y;
		vertices[v].coord3d[2] += blockState.drawOffset().z;

		vertices[v].coord3d[3] = f;

		vertices[v].normal[0] = normal.x;
		vertices[v].normal[1] = normal.y;
		vertices[v].normal[2] = normal.z;

		const gk::Color colorMultiplier = blockState.colorMultiplier();
		vertices[v].color[0] = colorMultiplier.r;
		vertices[v].color[1] = colorMultiplier.g;
		vertices[v].color[2] = colorMultiplier.b;
		vertices[v].color[3] = colorMultiplier.a;

		float U = (v == 0 || v == 3) ? U0 : U1;
		float V = (v >= 2) ? V0 : V1;
		vertices[v].texCoord[0] = gk::qlerp(blockTexCoords.x, blockTexCoords.x + blockTexCoords.sizeX, U);
		vertices[v].texCoord[1] = gk::qlerp(blockTexCoords.y, blockTexCoords.y + blockTexCoords.sizeY, V);

		if (Config::isSmoothLightingEnabled)
			vertices[v].lightValue[0] = getLightForVertex(Light::Sun, x, y, z, *neighbourOfs[v], normal, job.chunkData);
		else
			vertices[v].lightValue[0] = job.chunkData.getSunlight(sx, sy, sz);

		if (Config::isSmoothLightingEnabled && !blockState.isLightSource())
			vertices[v].lightValue[1] = getLightForVertex(Light::Torch, x, y, z, *neighbourOfs[v], normal, job.chunkData);
		else if (blockState.isOpaque())
			vertices[v].lightValue[1] = job.chunkData.getTorchlight(sx, sy, sz);
		else
			vertices[v].lightValue[1] = job.chunkData.getTorchlight(x, y, z);

		vertices[v].ambientOcclusion = getAmbientOcclusion(x, y, z, *neighbourOfs[v], normal, job.chunkData);
	}

	auto addVertex = [&](u8 v) {
		if (Config::ambientOcclusion != 1 || blockState.isLightSource())
			vertices[v].ambientOcclusion = 5;

		if (blockState.drawType() == BlockDrawType::Liquid)
			job.vertices[Layer::Liquid].emplace_back(vertices[v]);
		else if (blockState.drawType() == BlockDrawType::Glass)
			job.vertices[Layer::Glass].emplace_back(vertices[v]);
		else if (blockState.colorMultiplier() != gk::Color::White)
			job.vertices[Layer::NoMipMap].emplace_back(vertices[v]);
		else
			job.vertices[Layer::Solid].emplace_back(vertices[v]);
	};

	// Flipping quad to fix anisotropy issue
	if (vertices[0].ambientOcclusion + vertices[2].ambientOcclusion >
			vertices[1].ambientOcclusion + vertices[3].ambientOcclusion) {
		addVertex(0);
		addVertex(1);
		addVertex(2);
		addVertex(2);
		addVertex(3);
		addVertex(0);
	} else {
		addVertex(0);
		addVertex(1);
		addVertex(3);
		addVertex(3);
		addVertex(1);
		addVertex(2);
	}
}

inline void ChunkMeshBuilder::addCross(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job, const BlockState &blockState) {
	glm::vec3 vertexPos[nVertsPerCube]{
		{0, 0, 0},
		{1, 0, 0},
		{0, 1, 0},
		{1, 1, 0},
		{0, 0, 1},
		{1, 0, 1},
		{0, 1, 1},
		{1, 1, 1},
	};

	const glm::vec3 *const faceVertices[nCrossFaces][nVertsPerFace]{
		{&vertexPos[crossVerts[0][0]], &vertexPos[crossVerts[0][1]],
		 &vertexPos[crossVerts[0][2]], &vertexPos[crossVerts[0][3]]},
		{&vertexPos[crossVerts[1][0]], &vertexPos[crossVerts[1][1]],
		 &vertexPos[crossVerts[1][2]], &vertexPos[crossVerts[1][3]]},
	};

	const std::string &texture = blockState.tiles().getTextureForFace(0);
	const gk::FloatRect &blockTexCoords = job.textureAtlas->getTexCoords(texture);

	float faceTexCoords[nVertsPerFace][nCoordsPerUV] = {
		{blockTexCoords.x,                        blockTexCoords.y + blockTexCoords.sizeY},
		{blockTexCoords.x + blockTexCoords.sizeX, blockTexCoords.y + blockTexCoords.sizeY},
		{blockTexCoords.x + blockTexCoords.sizeX, blockTexCoords.y},
		{blockTexCoords.x,                        blockTexCoords.y},
	};

	for (int f = 0; f < nCrossFaces ; ++f) {
		Vertex vertices[nVertsPerFace];
		for (int v = 0 ; v < nVertsPerFace ; ++v) {
			vertices[v].coord3d[0] = x + faceVertices[f][v]->x + blockState.drawOffset().x;
			vertices[v].coord3d[1] = y + faceVertices[f][v]->y + blockState.drawOffset().y;
			vertices[v].coord3d[2] = z + faceVertices[f][v]->z + blockState.drawOffset().z;
			vertices[v].coord3d[3] = 6;

			vertices[v].normal[0] = 0;
			vertices[v].normal[1] = 0;
			vertices[v].normal[2] = 0;

			const gk::Color colorMultiplier = blockState.colorMultiplier();
			vertices[v].color[0] = colorMultiplier.r;
			vertices[v].color[1] = colorMultiplier.g;
			vertices[v].color[2] = colorMultiplier.b;
			vertices[v].color[3] = colorMultiplier.a;

			vertices[v].texCoord[0] = faceTexCoords[v][0];
			vertices[v].texCoord[1] = faceTexCoords[v][1];

			vertices[v].lightValue[0] = job.chunkData.getSunlight(x, y, z);
			vertices[v].lightValue[1] = job.chunkData.getTorchlight(x, y, z);

			vertices[v].ambientOcclusion = 5;
		}

		job.vertices[Layer::Flora].emplace_back(vertices[0]);
		job.vertices[Layer::Flora].emplace_back(vertices[1]);
		job.vertices[Layer::Flora].emplace_back(vertices[3]);
		job.vertices[Layer::Flora].emplace_back(vertices[3]);
		job.vertices[Layer::Flora].emplace_back(vertices[1]);
		job.vertices[Layer::Flora].emplace_back(vertices[2]);
	}
}

// Based on this article: https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
inline u8 ChunkMeshBuilder::getAmbientOcclusion(s8f x, s8f y, s8f z, const gk::Vector3i &offset, const gk::Vector3i &normal, const ChunkData &chunk) {
	gk::Vector3i minOffset{
		(normal.x != 0) ? offset.x : 0,
		(normal.y != 0) ? offset.y : 0,
		(normal.z != 0) ? offset.z : 0
	};

	const BlockState *blocks[4] = {
		chunk.getBlockState(x + minOffset.x, y + minOffset.y, z + offset.z),
		chunk.getBlockState(x + offset.x,    y + minOffset.y, z + minOffset.z),
		chunk.getBlockState(x + minOffset.x, y + offset.y,    z + minOffset.z),
		chunk.getBlockState(x + offset.x,    y + offset.y,    z + offset.z)
	};

	bool blockPresence[4] = {
		blocks[0] && blocks[0]->block().id() != 0 && blocks[0]->isOpaque(),
		blocks[1] && blocks[1]->block().id() != 0 && blocks[1]->isOpaque(),
		blocks[2] && blocks[2]->block().id() != 0 && blocks[2]->isOpaque(),
		blocks[3] && blocks[3]->block().id() != 0 && blocks[3]->isOpaque()
	};

	bool side1 = blockPresence[(minOffset.x != 0) ? 2 : 1];
	bool side2 = blockPresence[(minOffset.z != 0) ? 2 : 0];
	bool corner = blockPresence[3];

	return (side1 && side2) ? 0 : 3 - (side1 + side2 + corner);
}

inline u8 ChunkMeshBuilder::getLightForVertex(Light light, s8f x, s8f y, s8f z, const gk::Vector3i &offset, const gk::Vector3i &normal, const ChunkData &chunk) {
	std::function<s8(const ChunkData &chunk, s8, s8, s8)> getLight = [&](const ChunkData &chunk, s8 x, s8 y, s8 z) -> s8 {
		// if (x < 0)             return chunk->getSurroundingChunk(0) && chunk->getSurroundingChunk(0)->isInitialized() ? getLight(chunk->getSurroundingChunk(0), x + CHUNK_WIDTH, y, z) : -1;
		// if (x >= CHUNK_WIDTH)  return chunk->getSurroundingChunk(1) && chunk->getSurroundingChunk(1)->isInitialized() ? getLight(chunk->getSurroundingChunk(1), x - CHUNK_WIDTH, y, z) : -1;
		// if (y < 0)             return chunk->getSurroundingChunk(2) && chunk->getSurroundingChunk(2)->isInitialized() ? getLight(chunk->getSurroundingChunk(2), x, y + CHUNK_DEPTH, z) : -1;
		// if (y >= CHUNK_DEPTH)  return chunk->getSurroundingChunk(3) && chunk->getSurroundingChunk(3)->isInitialized() ? getLight(chunk->getSurroundingChunk(3), x, y - CHUNK_DEPTH, z) : -1;
		// if (z < 0)             return chunk->getSurroundingChunk(4) && chunk->getSurroundingChunk(4)->isInitialized() ? getLight(chunk->getSurroundingChunk(4), x, y, z + CHUNK_HEIGHT) : -1;
		// if (z >= CHUNK_HEIGHT) return chunk->getSurroundingChunk(5) && chunk->getSurroundingChunk(5)->isInitialized() ? getLight(chunk->getSurroundingChunk(5), x, y, z - CHUNK_HEIGHT) : -1;
        //
		// if (light == Light::Sun)
		// 	return chunk->isInitialized() ? chunk->lightmap().getSunlight(x, y, z) : -1;
		// else
		// 	return chunk->isInitialized() ? chunk->lightmap().getTorchlight(x, y, z) : -1;

		return (light == Light::Sun)
			? chunk.getSunlight(x, y, z)
			: chunk.getTorchlight(x, y, z);
	};

	gk::Vector3i minOffset{
		(normal.x != 0) ? offset.x : 0,
		(normal.y != 0) ? offset.y : 0,
		(normal.z != 0) ? offset.z : 0
	};

	gk::Vector3i surroundingBlocks[4]{
		{x + minOffset.x, y + minOffset.y, z + offset.z},
		{x + offset.x,    y + minOffset.y, z + minOffset.z},
		{x + minOffset.x, y + offset.y,    z + minOffset.z},
		{x + offset.x,    y + offset.y,    z + offset.z}
	};

	// Get light values for surrounding nodes
	s8 lightValues[4] = {
		getLight(chunk, surroundingBlocks[0].x, surroundingBlocks[0].y, surroundingBlocks[0].z),
		getLight(chunk, surroundingBlocks[1].x, surroundingBlocks[1].y, surroundingBlocks[1].z),
		getLight(chunk, surroundingBlocks[2].x, surroundingBlocks[2].y, surroundingBlocks[2].z),
		getLight(chunk, surroundingBlocks[3].x, surroundingBlocks[3].y, surroundingBlocks[3].z),
	};

	float count = 0, total = 0;
	for (u8 i = 0 ; i < 4 ; ++i) {
		// Fix light approximation
		// if (i == 3 && lightValues[i] > lightValues[0] && !lightValues[1] && !lightValues[2])
		// 	continue;

		// If the chunk is initialized, add the light value to the total
		// But only add dark blocks if AO is set on Smooth Lighting
		if (lightValues[i] != -1 && (Config::ambientOcclusion == 2 || lightValues[i] != 0)) {
			total += lightValues[i];
			++count;
		}
	}

	if (count)
		return total / count;
	else
		return 0;
}
