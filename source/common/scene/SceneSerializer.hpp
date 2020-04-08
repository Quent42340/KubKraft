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
#ifndef SCENESERIALIZER_HPP_
#define SCENESERIALIZER_HPP_

#include <gk/core/Debug.hpp>

#include <entt/entt.hpp>

#include "NetworkUtils.hpp"

class Scene;

class SceneSerializer {
	using Entity = entt::registry::entity_type;

	public:
		void serialize(sf::Packet &packet, const Scene &scene) const;
		void deserialize(sf::Packet &packet, Scene &scene);

	private:
		class OutputArchive {
			public:
				void operator()(Entity entity);

				template<typename T>
				void operator()(Entity entity, const T &value) {
					// gkDebug() << entity << (void *)&value << typeid(T).name();
					(*m_packet) << entity << value;
					// FIXME: It should be possible to check the type here and to create
					//        a defintion struct to serialize for some of them
					//        instead of sending the component
				}

				void setPacket(sf::Packet &packet) { m_packet = &packet; }

			private:
				sf::Packet *m_packet = nullptr;
		};

		class InputArchive {
			public:
				void operator()(Entity &entity);

				template<typename T>
				void operator()(Entity &entity, T &value) {
					(*m_packet) >> entity >> value;
					// gkDebug() << entity << (void *)&value << typeid(T).name();
				}

				void setPacket(sf::Packet &packet) { m_packet = &packet; }

			private:
				sf::Packet *m_packet = nullptr;
		};

		mutable OutputArchive m_outputArchive;
		mutable InputArchive m_inputArchive;
};

#endif // SCENESERIALIZER_HPP_
