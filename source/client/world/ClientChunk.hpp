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
#ifndef CLIENTCHUNK_HPP_
#define CLIENTCHUNK_HPP_

#include <gk/gl/Drawable.hpp>

#include "Chunk.hpp"
#include "ChunkBuilder.hpp"
#include "Config.hpp"

class TextureAtlas;

class ClientChunk : public Chunk {
	public:
		ClientChunk(s32 x, s32 y, s32 z, World &world, TextureAtlas &textureAtlas)
			: Chunk(x, y, z, world), m_textureAtlas(textureAtlas), m_builder{textureAtlas} {}

		void update();

		void drawLayer(gk::RenderTarget &target, gk::RenderStates states, u8 layer) const;

		bool hasBeenRequested() const { return m_hasBeenRequested; }
		void setHasBeenRequested(bool hasBeenRequested) { m_hasBeenRequested = hasBeenRequested; }

		bool isTooFar() const { return m_isTooFar; }
		void setTooFar(bool isTooFar) { m_isTooFar = isTooFar; }

		bool areAllNeighboursTooFar() const;

	private:
		TextureAtlas &m_textureAtlas;

		ChunkBuilder m_builder;

		std::array<gk::VertexBuffer, ChunkBuilder::layers> m_vbo{};
		std::array<std::size_t, ChunkBuilder::layers> m_verticesCount{};

		bool m_hasBeenRequested = false;
		bool m_isTooFar = false;
};

#endif // CLIENTCHUNK_HPP_
