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
#ifndef BLOCKMESHER_HPP_
#define BLOCKMESHER_HPP_

#include "ChunkMeshBuildingJob.hpp"

class BlockMesher {
	public:
		static void addBlock(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job,
		                     const BlockState &blockState, u16 blockParam);

		static void addBlock(s8f x, s8f y, s8f z, ChunkMeshBuildingJob &job,
		                     const BlockState &blockState, const gk::FloatBox &boundingBox,
		                     u8f orientation, const glm::mat3 &orientMatrix);

		static void addBlockFace(s8f x, s8f y, s8f z, s8f f, ChunkMeshBuildingJob &job,
		                         const BlockState &blockState, const gk::FloatBox &boundingBox,
		                         const gk::Vector3<s8f> &normal, const glm::vec3 *const vertexPos[4],
		                         const gk::Vector3<s8f> *const neighbourOfs[4]);
};

#endif // BLOCKMESHER_HPP_
