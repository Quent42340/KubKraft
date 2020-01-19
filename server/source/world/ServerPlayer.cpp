/*
 * =====================================================================================
 *
 *       Filename:  ServerPlayer.cpp
 *
 *    Description:
 *
 *        Created:  22/01/2019 23:11:46
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include "ServerPlayer.hpp"

ServerPlayer::ServerPlayer(Client &client) : m_client(client) {
	m_clientID = client.id;
}

