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
#include "BlockMesher.hpp"
#include "Config.hpp"
#include "LightUtils.hpp"

using namespace BlockGeometry;
using namespace LightUtils;

void BlockMesher::addBlock(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job,
                           const BlockState &blockState, u16 blockParam)
{
	const gk::FloatBox &boundingBox = blockState.boundingBox();

	u8f orientation = blockState.block().isRotatable()
		? (u8f)blockState.block().param().getParam(BlockParam::Rotation, blockParam) : 0;
	const glm::mat3 &orientMatrix = orientMatrices[orientation];

	addBlock(x, y, z, job, blockState, boundingBox, orientation, orientMatrix);
}

void BlockMesher::addBlock(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job,
                           const BlockState &blockState, const gk::FloatBox &boundingBox,
                           u8f orientation, const glm::mat3 &orientMatrix)
{
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
			vertexPos[i].x = float((i >> 0) & 1);
			vertexPos[i].y = float((i >> 1) & 1);
			vertexPos[i].z = float((i >> 2) & 1);
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
		const gk::Vector3<s8f> normal{s8f(glmNormal.x), s8f(glmNormal.y), s8f(glmNormal.z)};

		// Construct an array with the 4 vertex positions of this face
		glm::vec3 *faceVerts[nVertsPerFace]{
			&vertexPos[cubeVerts[f][0]], &vertexPos[cubeVerts[f][1]],
			&vertexPos[cubeVerts[f][2]], &vertexPos[cubeVerts[f][3]]
		};

		// Construct an array with the 4 vertex neighbours of this face
		// (as GameKit integer vectors)
		const gk::Vector3<s8f> corner0{s8f(vNeighbour[cubeVerts[f][0]].x), s8f(vNeighbour[cubeVerts[f][0]].y), s8f(vNeighbour[cubeVerts[f][0]].z)};
		const gk::Vector3<s8f> corner1{s8f(vNeighbour[cubeVerts[f][1]].x), s8f(vNeighbour[cubeVerts[f][1]].y), s8f(vNeighbour[cubeVerts[f][1]].z)};
		const gk::Vector3<s8f> corner2{s8f(vNeighbour[cubeVerts[f][2]].x), s8f(vNeighbour[cubeVerts[f][2]].y), s8f(vNeighbour[cubeVerts[f][2]].z)};
		const gk::Vector3<s8f> corner3{s8f(vNeighbour[cubeVerts[f][3]].x), s8f(vNeighbour[cubeVerts[f][3]].y), s8f(vNeighbour[cubeVerts[f][3]].z)};

		const gk::Vector3<s8f> *vFaceNeighbours[nVertsPerFace]{&corner0, &corner1, &corner2, &corner3};

		addBlockFace(x, y, z, f, job, blockState, boundingBox, normal, faceVerts, vFaceNeighbours);
	}
}

void BlockMesher::addBlockFace(s8f x, s8f y, s8f z, s8f f, ChunkMeshBuildingJob &job,
                              const BlockState &blockState,
                              const gk::FloatBox &boundingBox,
                              const gk::Vector3<s8f> &normal,
                              const glm::vec3 *const vertexPos[nVertsPerFace],
                              const gk::Vector3<s8f> *const neighbourOfs[nVertsPerFace])
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
		V0 = (f <= 3) ? 1.f - (boundingBox.z + boundingBox.sizeZ) : (f == 4) ? boundingBox.y :
		                1.f - (boundingBox.y + boundingBox.sizeY);
		U1 = (f == 0) ? 1.f - boundingBox.y : (f == 1) ? boundingBox.y + boundingBox.sizeY :
		     (f == 3) ? 1.f - boundingBox.x : boundingBox.x + boundingBox.sizeX;
		V1 = (f <= 3) ? 1.f - boundingBox.z : (f == 4) ? boundingBox.y + boundingBox.sizeY :
		                1.f - boundingBox.y;
	}

	// Prepare vertex information for VBO
	Vertex vertices[nVertsPerFace];
	for (s8f v = 0; v < nVertsPerFace; ++v) {
		if (blockState.drawType() == BlockDrawType::Cactus) {
			vertices[v].coord3d[0] = x + vertexPos[v]->x - boundingBox.x * (float)normal.x;
			vertices[v].coord3d[1] = y + vertexPos[v]->y - boundingBox.y * (float)normal.y;
			vertices[v].coord3d[2] = z + vertexPos[v]->z - boundingBox.z * (float)normal.z;
		}
		else {
			float blockHeight = vertexPos[v]->z;
			if (blockState.drawType() == BlockDrawType::Liquid) {
				const BlockState *topBlockState = job.chunkData.getBlockState(x, y, z + 1);
				if (f != BlockFace::Bottom && (!topBlockState || !topBlockState->block().id())) {
					blockHeight *= 14.f / 16.f;
				}
			}

			vertices[v].coord3d[0] = x + vertexPos[v]->x;
			vertices[v].coord3d[1] = y + vertexPos[v]->y;
			vertices[v].coord3d[2] = z + blockHeight;
		}

		vertices[v].coord3d[0] += blockState.drawOffset().x;
		vertices[v].coord3d[1] += blockState.drawOffset().y;
		vertices[v].coord3d[2] += blockState.drawOffset().z;

		vertices[v].coord3d[3] = f;

		vertices[v].normal[0] = (float)normal.x;
		vertices[v].normal[1] = (float)normal.y;
		vertices[v].normal[2] = (float)normal.z;

		const gk::Color colorMultiplier = blockState.colorMultiplier();
		vertices[v].color[0] = colorMultiplier.r;
		vertices[v].color[1] = colorMultiplier.g;
		vertices[v].color[2] = colorMultiplier.b;
		vertices[v].color[3] = colorMultiplier.a;

		float U = (v == 0 || v == 3) ? U0 : U1;
		float V = (v >= 2) ? V0 : V1;
		vertices[v].texCoord[0] = gk::qlerpf(blockTexCoords.x, blockTexCoords.x + blockTexCoords.sizeX, U);
		vertices[v].texCoord[1] = gk::qlerpf(blockTexCoords.y, blockTexCoords.y + blockTexCoords.sizeY, V);

		// Lighting-based implementation
		bool useAO = Config::ambientOcclusion == 2 && blockState.drawType() != BlockDrawType::Multibox;

		if (Config::isSmoothLightingEnabled)
			vertices[v].lightValue[0] = getLightForVertex(LightType::Sun, x, y, z, *neighbourOfs[v], normal, job.chunkData, useAO);
		else if (blockState.isOpaque())
			vertices[v].lightValue[0] = job.chunkData.getSunlight(sx, sy, sz);
		else
			vertices[v].lightValue[0] = job.chunkData.getSunlight(x, y, z);

		if (Config::isSmoothLightingEnabled && !blockState.isLightSource())
			vertices[v].lightValue[1] = getLightForVertex(LightType::Torch, x, y, z, *neighbourOfs[v], normal, job.chunkData, useAO);
		else if (blockState.isOpaque())
			vertices[v].lightValue[1] = job.chunkData.getTorchlight(sx, sy, sz);
		else
			vertices[v].lightValue[1] = job.chunkData.getTorchlight(x, y, z);

		vertices[v].ambientOcclusion = getAmbientOcclusion(x, y, z, *neighbourOfs[v], normal, job.chunkData);
	}

	// Fix basic ambient occlusion for multibox draw type
	if (blockState.drawType() == BlockDrawType::Multibox) {
		float aoValues[nVertsPerFace] = {
			vertices[0].ambientOcclusion,
			vertices[1].ambientOcclusion,
			vertices[2].ambientOcclusion,
			vertices[3].ambientOcclusion
		};

		for (u8f v = 0; v < nVertsPerFace; ++v) {
			float U = (v == 0 || v == 3) ? U0 : U1;
			float V = 1.f - ((v >= 2) ? V0 : V1);

			float a = gk::lerpf(aoValues[0], aoValues[1], U);
			float b = gk::lerpf(aoValues[3], aoValues[2], U);
			vertices[v].ambientOcclusion = gk::lerpf(a, b, V);
		}
	}

	auto addVertex = [&](u8 v) {
		if (Config::ambientOcclusion != 1 || blockState.isLightSource())
			vertices[v].ambientOcclusion = 4;

		if (blockState.drawType() == BlockDrawType::Liquid)
			job.vertices[ChunkMeshLayer::Liquid].emplace_back(vertices[v]);
		else if (blockState.drawType() == BlockDrawType::Glass)
			job.vertices[ChunkMeshLayer::Glass].emplace_back(vertices[v]);
		else if (blockState.colorMultiplier() != gk::Color::White)
			job.vertices[ChunkMeshLayer::NoMipMap].emplace_back(vertices[v]);
		else
			job.vertices[ChunkMeshLayer::Solid].emplace_back(vertices[v]);
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

	job.totalVertexCount += 6;
}

