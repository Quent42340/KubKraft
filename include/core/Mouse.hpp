/*
 * =====================================================================================
 *
 *       Filename:  Mouse.hpp
 *
 *    Description:
 *
 *        Created:  20/12/2014 15:38:51
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef MOUSE_HPP_
#define MOUSE_HPP_

#include "IntTypes.hpp"
#include "Vector2.hpp"
#include "Window.hpp"

class Mouse {
	public:
		static void update(const sf::Event &event);
		static void resetToWindowCenter();

		static Vector2i getPosition();
		static Vector2f getDelta() { return s_lastDelta; }

		static void setWindow(Window *window) { s_window = window; }

		static void setCursorVisible(bool isVisible);
		static void setCursorGrabbed(bool isGrabbed);

	private:
		static Window *s_window;

		static Vector2i s_lastMousePos;
		static Vector2f s_lastDelta;
};

#endif // MOUSE_HPP_
