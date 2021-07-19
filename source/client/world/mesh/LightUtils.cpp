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
#include "LightUtils.hpp"

// Based on this article: https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
u8 LightUtils::getAmbientOcclusion(s8f x, s8f y, s8f z,
                                   const gk::Vector3<s8f> &offset,
                                   const gk::Vector3<s8f> &normal,
                                   const ChunkData &chunk)
{
	gk::Vector3<s8f> minOffset{
		s8f((normal.x != 0) ? offset.x : 0),
		s8f((normal.y != 0) ? offset.y : 0),
		s8f((normal.z != 0) ? offset.z : 0)
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

	return u8((side1 && side2) ? 0 : 3 - (side1 + side2 + corner));
}

u8 LightUtils::getLightForVertex(LightType lightType, s8f x, s8f y, s8f z,
                                 const gk::Vector3<s8f> &offset,
                                 const gk::Vector3<s8f> &normal,
                                 const ChunkData &chunk, bool useAO)
{
	gk::Vector3<s8f> minOffset{
		(normal.x != 0) ? offset.x : s8f(0),
		(normal.y != 0) ? offset.y : s8f(0),
		(normal.z != 0) ? offset.z : s8f(0)
	};

	gk::Vector3<s8f> surroundingBlocks[4]{
		{s8f(x + minOffset.x), s8f(y + minOffset.y), s8f(z + offset.z)},
		{s8f(x + offset.x),    s8f(y + minOffset.y), s8f(z + minOffset.z)},
		{s8f(x + minOffset.x), s8f(y + offset.y),    s8f(z + minOffset.z)},
		{s8f(x + offset.x),    s8f(y + offset.y),    s8f(z + offset.z)}
	};

	auto getLight = [&](const ChunkData &chunk, s8f x, s8f y, s8f z) -> s8 {
		return (lightType == LightType::Sun)
			? chunk.getSunlight(x, y, z)
			: chunk.getTorchlight(x, y, z);
	};

	// Get light values for surrounding blocks
	s8 lightValues[4] = {
		getLight(chunk, surroundingBlocks[0].x, surroundingBlocks[0].y, surroundingBlocks[0].z),
		getLight(chunk, surroundingBlocks[1].x, surroundingBlocks[1].y, surroundingBlocks[1].z),
		getLight(chunk, surroundingBlocks[2].x, surroundingBlocks[2].y, surroundingBlocks[2].z),
		getLight(chunk, surroundingBlocks[3].x, surroundingBlocks[3].y, surroundingBlocks[3].z),
	};

	u8 count = 0, total = 0;
	for (u8 i = 0 ; i < 4 ; ++i) {
		// Fix light approximation
		if (i == 3 && lightValues[i] > lightValues[0] && !lightValues[1] && !lightValues[2])
			continue;

		// If the chunk is initialized, add the light value to the total
		// But only add dark blocks if AO is set on Smooth Lighting
		if (lightValues[i] != -1 && (useAO || lightValues[i] != 0)) {
			total += (u8)lightValues[i];
			++count;
		}
	}

	if (count)
		return u8(total / count);
	else
		return 0;
}

