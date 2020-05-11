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
#ifndef TITLESCREENSTATE_HPP_
#define TITLESCREENSTATE_HPP_

#include <thread>

#include "InterfaceState.hpp"
#include "MenuWidget.hpp"

struct GuiScaleChangedEvent;
struct ServerOnlineEvent;

class TitleScreenState : public InterfaceState {
	public:
		TitleScreenState(u16 port = 4242);
		~TitleScreenState();

		void centerBackground();

		void init() override;

		void onEvent(const sf::Event &event) override;

		void update() override;

		void startSingleplayer(bool showLoadingState);
		void startMultiplayer(const std::string &host);

		void setTexturePack(const std::string &texturePack) { m_texturePack = texturePack; }

	private:
		void onGuiScaleChanged(const GuiScaleChangedEvent &event);
		void onServerOnlineEvent(const ServerOnlineEvent &event);

		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

		MenuWidget m_menuWidget{1, 3};

		gk::Image m_background{"texture-title_screen"};

		std::thread m_thread;

		u16 m_port = 4242;

		bool m_showLoadingState = false;

		bool m_isServerOnline = false;
		bool m_isServerLaunched = false;

		std::string m_texturePack;
};

#endif // TITLESCREENSTATE_HPP_
