/*
 * =====================================================================================
 *
 *       Filename:  ChunkBuilder.hpp
 *
 *    Description:
 *
 *        Created:  19/06/2018 22:26:07
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef CHUNKBUILDER_HPP_
#define CHUNKBUILDER_HPP_

#include <array>
#include <vector>

#include "IntTypes.hpp"
#include "Vertex.hpp"

class Block;
class Chunk;
class VertexBuffer;

class ChunkBuilder {
	public:
		static constexpr u8 layers = 3;

		std::array<std::size_t, layers> buildChunk(const Chunk &chunk, const std::array<VertexBuffer, layers> &vbo);

	private:
		void addFace(u8 x, u8 y, u8 z, u8 i, const Chunk &chunk, const Block *block, const Block *surroundingBlock);

		float getAverageTorchlight(u8 x, u8 y, u8 z, s8 offsetX, s8 offsetY, s8 offsetZ, const Chunk &chunk);
		float getAverageSunlight(u8 x, u8 y, u8 z, s8 offsetX, s8 offsetY, s8 offsetZ, const Chunk &chunk);

		float getSunlightForVertex(u8 x, u8 y, u8 z, u8 i, u8 j, const Chunk &chunk);
		float getTorchlightForVertex(u8 x, u8 y, u8 z, u8 i, u8 j, const Chunk &chunk);

		std::array<std::vector<Vertex>, layers> m_vertices;

		enum Face {
			Left,
			Right,
			Bottom,
			Top,
			Front,
			Back
		};

		enum Layer {
			Solid,
			Liquid,
			Other
		};
};

#endif // CHUNKBUILDER_HPP_
