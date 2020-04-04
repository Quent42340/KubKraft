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
#include "AnimationController.hpp"
#include "ClientPlayer.hpp"
#include "CollisionController.hpp"
#include "Scene.hpp"
#include "RenderingController.hpp"

Scene::Scene(ClientPlayer &player) : m_player(player) {
	m_controllers.emplace_back(new AnimationController);
	m_controllers.emplace_back(new CollisionController(player));
	m_controllers.emplace_back(new RenderingController);
}

void Scene::update() {
	for (auto &controller : m_controllers)
		controller->update(m_registry);
}

void Scene::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	if (!m_camera) return;

	// Subtract the camera position - see comment in ClientWorld::draw()
	gk::Vector3d cameraPosition = m_camera->getDPosition();
	states.transform.translate(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z);

	for (auto &controller : m_controllers)
		controller->draw(m_registry, target, states);
}

