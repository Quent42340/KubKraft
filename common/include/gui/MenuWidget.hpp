/*
 * =====================================================================================
 *
 *       Filename:  MenuWidget.hpp
 *
 *    Description:
 *
 *        Created:  28/06/2018 10:32:26
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef MENUWIDGET_HPP_
#define MENUWIDGET_HPP_

#include "TextButton.hpp"

class MenuWidget : public Widget {
	public:
		MenuWidget(u16 width, u16 height, Widget *parent = nullptr);

		void reset(u16 width, u16 height);

		void onEvent(const SDL_Event &event) override;

		TextButton &addButton(u16 x, u16 y, const std::string &text, const TextButton::CppCallback &callback);

	private:
		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

		static constexpr u16 s_verticalSpacing = 5;
		static constexpr u16 s_horizontalSpacing = 5;

		u16 m_width;
		u16 m_height;

		std::vector<TextButton> m_buttons;
};

#endif // MENUWIDGET_HPP_
