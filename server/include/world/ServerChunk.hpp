/*
 * =====================================================================================
 *
 *       Filename:  ServerChunk.hpp
 *
 *    Description:
 *
 *        Created:  12/01/2019 01:32:59
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef SERVERCHUNK_HPP_
#define SERVERCHUNK_HPP_

#include <gk/core/IntTypes.hpp>

#include "Config.hpp"
#include "Chunk.hpp"
#include "TerrainGenerator.hpp"

class ServerChunk : public Chunk {
	public:
		ServerChunk(s32 x, s32 y, s32 z) : Chunk(x, y, z) {}

		void update();
		void generate();

		bool isGenerated() const { return m_isGenerated; }
		void setGenerated(bool isGenerated) { m_isGenerated = isGenerated; } // FIXME

	private:
		bool m_hasChanged = false;
		bool m_isGenerated = false;

		TerrainGenerator m_terrainGenerator;
};

#endif // SERVERCHUNK_HPP_
