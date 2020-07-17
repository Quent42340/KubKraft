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
#ifndef GAMETIME_HPP_
#define GAMETIME_HPP_

#include <gk/graphics/Color.hpp>

class Sky;

class GameTime {
	public:
		static constexpr float daySpeed = 1.f;
		static constexpr u32 dayLength = 24000;
		static constexpr u32 dayStartOffset = 3000;

		// Note: These 3 functions are only needed in the client
		static float getCurrentTime(float offset = 0.f, float speed = 1.f);
		static float getSunlightIntensityFromTime(float time);
		static gk::Color getSkyColorFromTime(const Sky &sky, float time);

		static void incrementTicks();
		static void setTicks(u64 ticks) { s_ticks = ticks; }

		static u16 getTicksPerSecond() { return s_ticksPerSecond; }
		static u64 getTicks() { return s_ticks; }

		static u8 getCurrentHour() {
			return u64((s_ticks + dayStartOffset + 3000.f) / 1000.f) % 24;
		}

		static u8 getCurrentMinute() {
			return u64((s_ticks + dayStartOffset + 3000.f) / 1000.f * 60.0f) % 60;
		}

	private:
		static void updateTpsCounter();

		static u64 s_ticks;
		static u16 s_ticksPerSecond;
};

#endif // GAMETIME_HPP_
