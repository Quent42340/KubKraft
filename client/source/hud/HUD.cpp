/*
 * =====================================================================================
 *
 *       Filename:  HUD.cpp
 *
 *    Description:
 *
 *        Created:  06/07/2018 13:58:46
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include <glm/gtc/matrix_transform.hpp>

#include "ClientPlayer.hpp"
#include "Config.hpp"
#include "HUD.hpp"

HUD::HUD(ClientPlayer &player, ClientWorld &world, ClientCommandHandler &client)
	: m_client(client),
	m_hotbar(player.inventory()),
	m_blockCursor(player, world, client),
	m_debugOverlay(player)
{
	setScale(GUI_SCALE, GUI_SCALE, 1);

	// FIXME: Duplicated with LuaGUI
	m_shader.createProgram();
	m_shader.addShader(GL_VERTEX_SHADER, "resources/shaders/basic.v.glsl");
	m_shader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/basic.f.glsl");
	m_shader.linkProgram();

	m_orthoMatrix = glm::ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f);

	m_hotbar.setPosition(SCREEN_WIDTH / getScale().x / 2 - m_hotbar.width() / 2, SCREEN_HEIGHT / getScale().y - m_hotbar.height(), 0);

	m_blockInfoWidget.setPosition(SCREEN_WIDTH / getScale().x / 2 - m_blockInfoWidget.width() / 2, 2, 0);
}

void HUD::onEvent(const SDL_Event &event) {
	if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3)
		m_isDebugOverlayVisible ^= 1;

	m_hotbar.onEvent(event);
	m_blockCursor.onEvent(event, m_hotbar);
}

void HUD::update() {
	// FIXME: Shouldn't be called every tick
	m_hotbar.update();

	m_blockCursor.update(m_hotbar, false);

	if (m_isDebugOverlayVisible)
		m_debugOverlay.update();

	m_blockInfoWidget.update();

	if (m_blockCursor.currentBlock() != m_blockInfoWidget.currentBlock())
		m_blockInfoWidget.setCurrentBlock(m_blockCursor.currentBlock());
}

void HUD::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	target.draw(m_blockCursor, states);

	target.disableView();

	states.shader = &m_shader;
	states.projectionMatrix = m_orthoMatrix;
	states.viewMatrix = gk::Transform::Identity;
	states.vertexAttributes = gk::VertexAttribute::Only2d;

	states.transform *= getTransform();

	if (m_isDebugOverlayVisible)
		target.draw(m_debugOverlay, states);

	target.draw(m_blockInfoWidget, states);
	target.draw(m_hotbar, states);

	states.transform = gk::Transform::Identity;

	target.draw(m_crosshair, states);
}

