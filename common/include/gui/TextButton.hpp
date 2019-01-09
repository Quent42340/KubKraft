/*
 * =====================================================================================
 *
 *       Filename:  TextButton.hpp
 *
 *    Description:
 *
 *        Created:  28/06/2018 10:16:52
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef TEXTBUTTON_HPP_
#define TEXTBUTTON_HPP_

#include <functional>

#include <sol.hpp>

#include <gk/graphics/Image.hpp>
// #include <gk/graphics/Text.hpp>

#include "Text.hpp"
#include "Widget.hpp"

class TextButton : public Widget {
	public:
		using CppCallback = std::function<void(TextButton &)>;
		using LuaCallback = sol::function;
		TextButton(Widget *parent = nullptr);
		TextButton(const CppCallback &callback, Widget *parent = nullptr);

		void onEvent(const SDL_Event &event) override;

		const std::string &text() const { return m_text.text(); }
		void setText(const std::string &text);
		void setCallback(const CppCallback &callback) { m_cppCallback = callback; }
		void setCallback(const LuaCallback &callback) { m_luaCallback = callback; }
		void setEnabled(bool isEnabled) { m_isEnabled = isEnabled; }

	private:
		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

		gk::Image m_background{"texture-widgets"};
		gk::Image m_hoverBackground{"texture-widgets"};
		gk::Image m_disabledBackground{"texture-widgets"};

		Text m_text;
		// gk::Text m_text;
		// gk::Text m_textShadow;

		CppCallback m_cppCallback;
		LuaCallback m_luaCallback;

		// FIXME: Replace these by an enum State
		bool m_isHovered = false;
		bool m_isEnabled = true;
};

#endif // TEXTBUTTON_HPP_
