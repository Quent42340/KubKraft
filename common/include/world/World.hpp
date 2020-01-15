/*
 * =====================================================================================
 *
 *       Filename:  World.hpp
 *
 *    Description:
 *
 *        Created:  16/12/2014 15:28:02
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef WORLD_HPP_
#define WORLD_HPP_

#include <memory>

#include "Chunk.hpp"

class World {
	public:
		virtual ~World() = default;

		virtual BlockData *getBlockData(int x, int y, int z) const = 0;

		virtual u16 getBlock(int x, int y, int z) const = 0;
		virtual void setBlock(int x, int y, int z, u16 id) const = 0;
		virtual u16 getData(int x, int y, int z) const = 0;
		virtual void setData(int x, int y, int z, u16 id) const = 0;

		static bool isReloadRequested;
};

#endif // WORLD_HPP_
