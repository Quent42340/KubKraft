/*
 * =====================================================================================
 *
 *       Filename:  Player.cpp
 *
 *    Description:
 *
 *        Created:  22/01/2019 23:46:58
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include "Network.hpp"
#include "Player.hpp"

void Player::serialize(sf::Packet &packet) const {
	packet << m_x << m_y << m_z << m_inventory;
}

void Player::deserialize(sf::Packet &packet) {
	packet >> m_x >> m_y >> m_z >> m_inventory;
}

