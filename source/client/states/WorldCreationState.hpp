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
#ifndef WORLDCREATIONSTATE_HPP_
#define WORLDCREATIONSTATE_HPP_

#include "InterfaceState.hpp"
#include "TextButton.hpp"
#include "TextInput.hpp"

class TitleScreenState;

class WorldCreationState : public InterfaceState {
	public:
		WorldCreationState(TitleScreenState *titleScreen, const std::string &originalName = "");

		void onEvent(const sf::Event &event) override;

		void update() override;

	private:
		void updateWidgetPosition();

		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

		TextInput m_textInput;

		TextButton m_createButton;
		TextButton m_cancelButton;

		Text m_errorText;
};

#endif // WORLDCREATIONSTATE_HPP_
