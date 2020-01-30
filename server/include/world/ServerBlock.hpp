/*
 * =====================================================================================
 *
 *       Filename:  ServerBlock.hpp
 *
 *    Description:
 *
 *        Created:  25/01/2019 00:20:34
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef SERVERBLOCK_HPP_
#define SERVERBLOCK_HPP_

#include "Block.hpp"

class Client;
class Server;
class ServerPlayer;

class ServerBlock : public Block {
	public:
		ServerBlock(u32 id, const std::string &textureFilename, const std::string &name, const std::string &label)
			: Block(id, textureFilename, name, label) {}

		void onTick(const glm::ivec3 &, std::unordered_map<u16, ServerPlayer> &, Chunk &, World &, Server &) const;
		bool onBlockActivated(const glm::ivec3 &pos, Player &player, World &world, Client &client) const;

		bool canUpdate() const { return m_onTick.valid(); }

		void setOnBlockActivated(const sol::function &function) { m_onBlockActivated = function; }
		void setOnTick(const sol::function &function) { m_onTick = function; m_canUpdate = m_onTick.valid(); }

	private:
		sol::unsafe_function m_onBlockActivated;
		sol::unsafe_function m_onTick;
		mutable bool m_onTickEnabled = true;
};

#endif // SERVERBLOCK_HPP_
