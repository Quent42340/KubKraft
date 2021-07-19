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
#include "MultiboxMesher.hpp"

using namespace BlockGeometry;

void MultiboxMesher::addMultibox(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job,
                                 const BlockState &blockState, u16 blockParam)
{
	u8f orientation = blockState.block().isRotatable()
		? (u8f)blockState.block().param().getParam(BlockParam::Rotation, blockParam) : 0;
	const glm::mat3 &orientMatrix = orientMatrices[orientation];

	if (blockState.multiboxType() != BlockState::WallMounted) {
		for (auto &it : blockState.multibox())
			BlockMesher::addBlock(x, y, z, job, blockState, it, orientation, orientMatrix);

		if (blockState.multiboxType() == BlockState::Connected) {
			u16 neighbours[6 + 1] = {
				job.chunkData.getBlockID(x - 1, y,     z),
				job.chunkData.getBlockID(x + 1, y,     z),
				job.chunkData.getBlockID(x,     y - 1, z),
				job.chunkData.getBlockID(x,     y + 1, z),
				job.chunkData.getBlockID(x,     y,     z - 1),
				job.chunkData.getBlockID(x,     y,     z + 1),

				job.chunkData.getBlockID(x, y, z),
			};

			for (int i = 0 ; i < 6 ; ++i) {
				if (neighbours[i] == neighbours[6]) {
					auto &boxes = blockState.connectedMultibox()[i];
					for (auto &it : boxes)
						BlockMesher::addBlock(x, y, z, job, blockState, it, orientation, orientMatrix);
				}
			}
		}
	}
	else {
		if (blockState.block().param().hasParam(BlockParam::WallMounted)) {
			u8 wall = (u8)blockState.block().param().getParam(BlockParam::WallMounted, blockParam);
			auto &boxes = blockState.wallMountedBoxes((BlockState::WallMountedBoxType)wall);
			if (!boxes.empty())
				for (auto &it : boxes)
					BlockMesher::addBlock(x, y, z, job, blockState, it, orientation, orientMatrix);
			else {
				gkError() << "Failed to build mesh for block" << blockState.block().stringID() << "at" << (int)x << (int)y << (int)z;

				static const std::string wallMountedBoxType[4] = {
					"wall_top",
					"wall_bottom",
					"wall_side",
					"invalid"
				};

				gkError() << ("Reason: Wall '" + wallMountedBoxType[wall] + "' not defined").c_str();
			}
		}
		else {
			gkError() << "Failed to build mesh for block" << blockState.block().stringID() << "at" << (int)x << (int)y << (int)z;
			gkError() << "Reason: Uses wallmounted subboxes without allocating block param accordingly";
		}
	}
}
