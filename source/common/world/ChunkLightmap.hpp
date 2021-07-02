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
#ifndef CHUNKLIGHTMAP_HPP_
#define CHUNKLIGHTMAP_HPP_

#include <optional>
#include <queue>

#include <gk/core/IntTypes.hpp>

#include "EngineConfig.hpp"

struct LightNode {
	LightNode(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	int x;
	int y;
	int z;
};

struct LightRemovalNode {
	LightRemovalNode(int _x, int _y, int _z, int _value) : x(_x), y(_y), z(_z), value(_value) {}
	int x;
	int y;
	int z;
	int value;
};

class Chunk;

class ChunkLightmap {
	public:
		ChunkLightmap(Chunk *chunk);

		bool addTorchlight(int x, int y, int z, u8 val, bool raw = false);
		bool addSunlight(int x, int y, int z, u8 val, bool raw = false);
		bool removeTorchlight(int x, int y, int z);
		bool removeSunlight(int x, int y, int z);

		void updateLights();
		void updateTorchlight();
		void updateSunlight();

		std::optional<u8> tryGetLightData(int x, int y, int z) const;
		u8 getLightDataRaw(int x, int y, int z) const { return m_lightMap[z][y][x]; }
		u8 getLightData(int x, int y, int z) const;
		u8 getSunlight(int x, int y, int z) const;
		u8 getTorchlight(int x, int y, int z) const;

		void setLightDataRaw(int x, int y, int z, u8 val);
		bool setLightData(int x, int y, int z, u8 val);
		bool setSunlight(int x, int y, int z, u8 val);

		bool hasChanged() const { return m_hasChanged; }
		void resetChangedFlag() { m_hasChanged = false; }

		using LightMapArray = u8[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH];
		const LightMapArray &data() const { return m_lightMap; }

	private:
		bool setTorchlight(int x, int y, int z, u8 val);

		bool setSunlightRaw(int x, int y, int z, u8 val);
		bool setTorchlightRaw(int x, int y, int z, u8 val);

		Chunk *m_chunk = nullptr;

		LightMapArray m_lightMap;

		std::queue<LightNode> m_torchlightBfsQueue;
		std::queue<LightNode> m_sunlightBfsQueue;
		std::queue<LightRemovalNode> m_torchlightRemovalBfsQueue;
		std::queue<LightRemovalNode> m_sunlightRemovalBfsQueue;

		bool m_hasChanged = false;
};

#endif // CHUNKLIGHTMAP_HPP_
