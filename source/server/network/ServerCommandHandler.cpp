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
#include "AnimationComponent.hpp"
#include "BlockData.hpp"
#include "DrawableDef.hpp"
#include "GameTime.hpp"
#include "NetworkComponent.hpp"
#include "PlayerList.hpp"
#include "Registry.hpp"
#include "ScriptEngine.hpp"
#include "Server.hpp"
#include "ServerBlock.hpp"
#include "ServerCommandHandler.hpp"
#include "ServerItem.hpp"
#include "WorldController.hpp"

void ServerCommandHandler::sendServerTick(const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::ServerTick << (sf::Uint64)GameTime::getTicks();

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendServerClosed(const std::string &message, ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::ServerClosed << message;

	if (!client) {
		m_server.sendToAllClients(packet);
		m_server.disconnectAllClients();
	}
	else {
		client->tcpSocket->send(packet);
		m_server.disconnectClient(*client);
	}
}

void ServerCommandHandler::sendChunkUnload(s32 chunkX, s32 chunkY, s32 chunkZ, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::ChunkUnload << chunkX << chunkY << chunkZ;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendBlockDataUpdate(s32 x, s32 y, s32 z, const BlockData *blockData, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::BlockDataUpdate << x << y << z << blockData->meta;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendBlockInvUpdate(s32 x, s32 y, s32 z, const Inventory &inventory, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::BlockInvUpdate << x << y << z << inventory;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendPlayerPosUpdate(u16 clientID, bool isTeleportation, const ClientInfo *client) const {
	const ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (player) {
		Network::Packet packet;
		packet << Network::Command::PlayerPosUpdate;
		packet << clientID;
		packet << player->x() << player->y() << player->z();
		packet << isTeleportation;

		if (!client)
			m_server.sendToAllClients(packet);
		else
			client->tcpSocket->send(packet);
	}
	else
		gkError() << ("Failed to send position update for player " + std::to_string(clientID) + ": Player not found").c_str();
}

void ServerCommandHandler::sendPlayerRotUpdate(u16 clientID, const ClientInfo *client) const {
	const ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (player) {
		Network::Packet packet;
		packet << Network::Command::PlayerRotUpdate;
		packet << clientID;
		packet << player->cameraYaw() << player->cameraPitch();

		if (!client)
			m_server.sendToAllClients(packet);
		else
			client->tcpSocket->send(packet);
	}
	else
		gkError() << ("Failed to send rotation update for player " + std::to_string(clientID) + ": Player not found").c_str();
}

void ServerCommandHandler::sendPlayerInvUpdate(u16 clientID, const ClientInfo *client) const {
	ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (player) {
		Network::Packet packet;
		packet << Network::Command::PlayerInvUpdate;
		packet << clientID << player->inventory() << player->heldItemSlot();

		if (!client)
			m_server.sendToAllClients(packet);
		else
			client->tcpSocket->send(packet);
	}
	else
		gkError() << ("Failed to send inventory update for player " + std::to_string(clientID) + ": Player not found").c_str();
}

void ServerCommandHandler::sendPlayerChangeDimension(u16 clientID, s32 x, s32 y, s32 z, u16 dimension, const ClientInfo *client) const {
	ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (player) {
		Network::Packet packet;
		packet << Network::Command::PlayerChangeDimension;
		packet << clientID << x << y << z << dimension;

		if (client) {
			client->tcpSocket->send(packet);

			// FIXME: sendPlayerChangeDimension shouldn't be exposed to Lua
			//        Instead, there should be a changePlayerDimension function that sends
			//        the packet above + the entities (instead of doing that here)
			//        also, it should change player dimension and position accordingly
			m_worldController.getWorld(dimension).scene().sendEntities(*client);

			player->setPosition(x + 0.5, y + 0.5, z + 0.5);
			player->setDimension(dimension);
			player->clearLoadedChunks();
		}
		else
			m_server.sendToAllClients(packet);
	}
	else
		gkError() << ("Failed to send dimension change for player " + std::to_string(clientID) + ": Player not found").c_str();
}

void ServerCommandHandler::sendChatMessage(u16 clientID, const std::string &message, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::ChatMessage << clientID << message;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendEntitySpawn(entt::entity entityID, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::EntitySpawn << entityID;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::sendEntityDespawn(entt::entity entityID, const ClientInfo *client) const {
	Network::Packet packet;
	packet << Network::Command::EntityDespawn << entityID;

	if (!client)
		m_server.sendToAllClients(packet);
	else
		client->tcpSocket->send(packet);
}

void ServerCommandHandler::setupCallbacks() {
	m_server.setConnectionCallback([this](ClientInfo &client, Network::Packet &connectionPacket) {
		std::string username;
		connectionPacket >> username;

		if (!gk::regexMatch(username, "^[A-Za-z0-9_]+$")) {
			sendServerClosed("Invalid username", &client);
			return;
		}

		ServerPlayer *player = m_players.connectPlayer(username, client, m_server.isSingleplayer());
		if (!player) {
			sendServerClosed("User is already online", &client);
			return;
		}

		// Try to find a valid spawn point (WIP)
		if (player->isNewPlayer()) {
			// FIXME: Default dimension hardcoded here
			ServerWorld &world = m_worldController.getWorld(0);
			Heightmap &heightmap = world.heightmap();

			player->setPosition(0, 0, 100);

			// FIXME
			// bool hasFoundPosition = false;
			// for (s32 spawnChunkX = 0 ; spawnChunkX < 16 && !hasFoundPosition ; ++spawnChunkX) {
			// 	for (s32 spawnChunkY = 0 ; spawnChunkY < 16 && !hasFoundPosition ; ++spawnChunkY) {
			// 		for(int y = 0 ; y < CHUNK_DEPTH && !hasFoundPosition ; y++) {
			// 			for(int x = 0 ; x < CHUNK_WIDTH ; x++) {
			// 				int maxChunkZ = heightmap.getHighestChunkAt(x + spawnChunkX * CHUNK_WIDTH, y + spawnChunkY * CHUNK_DEPTH);
			// 				int worldZ = heightmap.getHighestBlockAt(x + spawnChunkX * CHUNK_WIDTH, y + spawnChunkY * CHUNK_DEPTH) + 1;
			// 				int z = gk::pmod(worldZ, CHUNK_WIDTH);
            //
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX - 1, spawnChunkY,     maxChunkZ));
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX + 1, spawnChunkY,     maxChunkZ));
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX,     spawnChunkY - 1, maxChunkZ));
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX,     spawnChunkY + 1, maxChunkZ));
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX,     spawnChunkY,     maxChunkZ - 1));
			// 				world.generateChunk(world.getOrCreateChunk(spawnChunkX,     spawnChunkY,     maxChunkZ + 1));
            //
			// 				ServerChunk &chunk = world.getOrCreateChunk(spawnChunkX, spawnChunkY, maxChunkZ);
			// 				world.generateChunk(chunk);
            //
			// 				const BlockState *blockBelow = chunk.getBlockState(x, y, z - 1);
			// 				const Block &blockFeet = m_registry.getBlock(chunk.getBlock(x, y, z));
			// 				const Block &blockHead = m_registry.getBlock(chunk.getBlock(x, y, z + 1));
            //
			// 				if (blockFeet.id() == 0 && blockHead.id() == 0
			// 				&& blockBelow && blockBelow->isCollidable()
			// 				&& blockBelow->drawType() != BlockDrawType::Leaves)
			// 				{
			// 					player->setPosition(x + spawnChunkX * CHUNK_WIDTH + .5, y + spawnChunkY * CHUNK_DEPTH + .5, worldZ + .2);
			// 					hasFoundPosition = true;
			// 					break;
			// 				}
			// 			}
			// 		}
			// 	}
			// }
            //
			// if (!hasFoundPosition)
			// 	gkError() << "Can't find a good position for the player";

			player->setHeldItemSlot(0);
		}

		// Send the registry
		Network::Packet packet;
		packet << Network::Command::RegistryData;
		m_registry.serialize(packet);
		client.tcpSocket->send(packet);

		// Send already connected players to the new client
		for (auto &it : m_players) {
			Network::Packet spawnPacket;
			spawnPacket << Network::Command::PlayerSpawn << it.first;
			spawnPacket << it.second.x() << it.second.y() << it.second.z() << it.second.dimension() << it.second.name();
			spawnPacket << it.second.cameraYaw() << it.second.cameraPitch();
			client.tcpSocket->send(spawnPacket);
		}

		// Triggers the 'PlayerConnected' Lua event
		if (player->isNewPlayer())
			m_scriptEngine.luaCore().onEvent(LuaEventType::PlayerConnected, glm::ivec3{player->x(), player->y(), player->z()}, player, client, *this);

		// Send inventory
		sendPlayerInvUpdate(client.id, &client);

		// Send spawn packet to all clients for this player
		Network::Packet spawnPacket;
		spawnPacket << Network::Command::PlayerSpawn << client.id;
		spawnPacket << player->x() << player->y() << player->z() << player->dimension() << player->name();
		spawnPacket << player->cameraYaw() << player->cameraPitch();
		m_server.sendToAllClients(spawnPacket);

		// Send entities to the client
		m_worldController.getWorld(player->dimension()).scene().sendEntities(client);
	});

	m_server.setCommandCallback(Network::Command::ClientDisconnect, [this](ClientInfo &client, Network::Packet &) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			player->clearLoadedChunks();
			m_players.disconnectPlayer(client.playerName);
		}
	});

	m_server.setCommandCallback(Network::Command::ChunkUnload, [this](ClientInfo &client, Network::Packet &packet) {
		s32 cx, cy, cz;
		packet >> cx >> cy >> cz;

		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			player->removeLoadedChunk(gk::Vector3i{cx, cy, cz});
		}
		else
			gkError() << ("Failed to unload chunk for player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerInvUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			packet >> player->inventory();
		}
		else
			gkError() << ("Failed to update inventory of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerPosUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		double x, y, z;
		packet >> x >> y >> z;

		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			player->setPosition(x, y, z);
		}
		else
			gkError() << ("Failed to update position of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerRotUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		float yaw, pitch;
		packet >> yaw >> pitch;

		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			player->setRotation(yaw, pitch);
		}
		else
			gkError() << ("Failed to update rotation of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerPlaceBlock, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			s32 x, y, z;
			u32 block;
			packet >> x >> y >> z >> block;

			ServerWorld &world = getWorldForClient(client.id);
			world.setData(x, y, z, u16(block >> 16));
			world.setBlock(x, y, z, u16(block & 0xffff));

			const BlockState *blockState = world.getBlockState(x, y, z);
			if (!blockState) return;

			m_scriptEngine.luaCore().onEvent(LuaEventType::BlockPlaced, glm::ivec3{x, y, z}, *blockState, *player, world, client, *this);

			Network::Packet answer;
			answer << Network::Command::BlockUpdate << x << y << z << block;
			m_server.sendToAllClients(answer);
		}
		else
			gkError() << ("Failed to place block using player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerDigBlock, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			s32 x, y, z;
			packet >> x >> y >> z;

			ServerWorld &world = getWorldForClient(client.id);
			const BlockState *blockState = world.getBlockState(x, y, z);
			if (!blockState) return;
			world.setBlock(x, y, z, 0);

			m_scriptEngine.luaCore().onEvent(LuaEventType::BlockDigged, glm::ivec3{x, y, z}, *blockState, *player, world, client, *this);

			Network::Packet answer;
			answer << Network::Command::BlockUpdate << x << y << z << u32(0);
			m_server.sendToAllClients(answer);
		}
		else
			gkError() << ("Failed to dig block using player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerHeldItemChanged, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			u8 hotbarSlot;
			u16 itemID;
			packet >> hotbarSlot >> itemID;
			if (player->inventory().getStack(hotbarSlot, 0).item().id() != itemID)
				gkWarning() << "PlayerHeldItemChanged:" << "Desync of item ID between client and server";

			player->setHeldItemSlot(hotbarSlot);
		}
		else
			gkError() << ("Failed to change held item of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerReady, [this](ClientInfo &client, Network::Packet &) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			player->setReady(true);
		}
		else
			gkError() << ("Failed to change held item of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::PlayerChunkPosUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			s32 chunkX, chunkY, chunkZ;
			packet >> chunkX >> chunkY >> chunkZ;

			ServerWorld &world = getWorldForClient(client.id);
			world.updatePlayerChunks(*player, chunkX, chunkY, chunkZ);
		}
		else
			gkError() << ("Failed to update chunk position of player " + std::to_string(client.id) + ": Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::BlockActivated, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			s32 x, y, z;
			packet >> x >> y >> z >> client.screenWidth >> client.screenHeight >> client.guiScale;

			ServerWorld &world = getWorldForClient(client.id);

			u16 id = world.getBlock(x, y, z);
			ServerBlock &block = (ServerBlock &)(m_registry.getBlock(id));
			bool hasBeenActivated = block.onBlockActivated({x, y, z}, *player, world, client, *this);

			if (hasBeenActivated)
				m_scriptEngine.luaCore().onEvent(LuaEventType::BlockActivated, glm::ivec3{x, y, z}, block, *player, world, client, *this);
		}
		else
			gkError() << ("Failed to activate block using player '" + client.playerName + "': Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::BlockInvUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		gk::Vector3<s32> pos;
		packet >> pos.x >> pos.y >> pos.z;

		BlockData *data = getWorldForClient(client.id).getBlockData(pos.x, pos.y, pos.z);
		if (data)
			packet >> data->inventory;
		else
			gkError() << "BlockInvUpdate: No block data found at" << pos.x << pos.y << pos.z;
	});

	m_server.setCommandCallback(Network::Command::BlockDataUpdate, [this](ClientInfo &client, Network::Packet &packet) {
		gk::Vector3<s32> pos;
		packet >> pos.x >> pos.y >> pos.z;

		BlockData *data = getWorldForClient(client.id).getBlockData(pos.x, pos.y, pos.z);
		if (data) {
			packet >> data->meta;
		}
		else
			gkError() << "BlockDataUpdate: No block data found at" << pos.x << pos.y << pos.z;
	});

	m_server.setCommandCallback(Network::Command::ItemActivated, [this](ClientInfo &client, Network::Packet &packet) {
		ServerPlayer *player = m_players.getPlayerFromClientID(client.id);
		if (player) {
			s32 x, y, z;
			packet >> x >> y >> z >> client.screenWidth >> client.screenHeight >> client.guiScale;

			ServerWorld &world = getWorldForClient(client.id);

			u16 id = world.getBlock(x, y, z);
			ServerBlock &block = (ServerBlock &)(m_registry.getBlock(id));

			ServerItem &item = (ServerItem &)player->heldItemStack().item();
			if (item.canBeActivated()) {
				bool hasBeenActivated = item.onItemActivated({x, y, z}, block, *player, world, client, *this);

				if (hasBeenActivated)
					m_scriptEngine.luaCore().onEvent(LuaEventType::ItemActivated, glm::ivec3{x, y, z}, block, *player, world, client, *this);
			}
		}
		else
			gkError() << ("Failed to activate item using player '" + client.playerName + "': Player not found").c_str();
	});

	m_server.setCommandCallback(Network::Command::ChatMessage, [this](ClientInfo &client, Network::Packet &packet) {
		std::string message;
		packet >> message;

		if (message[0] != '/' || (message.length() > 1 && message[1] == '/')) {
			if (message[0] == '/' && message.length() > 1 && message[1] == '/')
				sendChatMessage(client.id, message.substr(1));
			else
				sendChatMessage(client.id, message);
		}
		else {
			m_chatCommandHandler.parseCommand(message.substr(1), client);
		}
	});

	m_server.setCommandCallback(Network::Command::KeyPressed, [this](ClientInfo &client, Network::Packet &packet) {
		u16 keyID;
		packet >> keyID >> client.screenWidth >> client.screenHeight >> client.guiScale;

		m_registry.getKey(keyID).callback()(keyID, client);
	});
}

void ServerCommandHandler::setPlayerPosition(u16 clientID, s32 x, s32 y, s32 z) {
	ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (player)
		player->setPosition(x, y, z);
	else
		gkError() << ("Failed to set position for player " + std::to_string(clientID) + ": Player not found").c_str();
}

inline ServerWorld &ServerCommandHandler::getWorldForClient(u16 clientID) {
	ServerPlayer *player = m_players.getPlayerFromClientID(clientID);
	if (!player)
		throw EXCEPTION("Player instance not found for client", clientID);

	return m_worldController.getWorld(player->dimension());
}

void ServerCommandHandler::stopServer() const {
	m_server.setRunning(false);
}

// Please update 'docs/lua-api-cpp.md' if you change this
void ServerCommandHandler::initUsertype(sol::state &lua) {
	lua.new_usertype<ServerCommandHandler>("ServerCommandHandler",
		"send_player_change_dimension", &ServerCommandHandler::sendPlayerChangeDimension,
		"send_chat_message", &ServerCommandHandler::sendChatMessage
	);
}

