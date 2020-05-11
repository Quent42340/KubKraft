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
#include "TextButton.hpp"

TextButton::TextButton(Widget *parent) : Widget(200, 20, parent) {
	m_background.setClipRect(0, 66, 200, 20);
	m_hoverBackground.setClipRect(0, 86, 200, 20);
	m_disabledBackground.setClipRect(0, 46, 200, 20);
}

TextButton::TextButton(const CppCallback &callback, Widget *parent) : TextButton(parent) {
	m_cppCallback = callback;
}

void TextButton::onEvent(const sf::Event &event) {
	if (event.type == sf::Event::MouseMoved) {
		m_isHovered = isPointInWidget(event.mouseMove.x, event.mouseMove.y);

		if (m_isEnabled && m_isHovered && m_text.color() == gk::Color::White)
			m_text.setColor({255, 255, 160});
		else if (!m_isHovered && m_text.color() != gk::Color::White)
			m_text.setColor(gk::Color::White);
	}
	else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && m_isHovered && m_isEnabled) {
		if (m_cppCallback)
			m_cppCallback(*this);
		else if (m_luaCallback)
			m_luaCallback(*this);
	}
}

void TextButton::setText(const std::string &text) {
	m_text.setText(text);
	m_text.updateVertexBuffer();
	m_text.setPosition(m_width / 2 - m_text.getSize().x / 2,
			m_height / 2 - m_text.getSize().y / 2, 0);
}

void TextButton::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	states.transform *= getTransform();

	if (!m_isEnabled)
		target.draw(m_disabledBackground, states);
	else if (m_isHovered)
		target.draw(m_hoverBackground, states);
	else
		target.draw(m_background, states);

	target.draw(m_text, states);
}

