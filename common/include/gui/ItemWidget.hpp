/*
 * =====================================================================================
 *
 *       Filename:  ItemWidget.hpp
 *
 *    Description:
 *
 *        Created:  21/06/2018 01:10:13
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef ITEMWIDGET_HPP_
#define ITEMWIDGET_HPP_

#include <gk/graphics/Image.hpp>

#include "Cube.hpp"
#include "Inventory.hpp"
#include "Text.hpp"
#include "Widget.hpp"

class ItemWidget : public Widget {
	public:
		ItemWidget(Inventory &inventory, u16 x, u16 y, Widget *parent = nullptr);

		void update() override;

		const ItemStack &stack() const { return m_inventory.getStack(m_x, m_y); }
		void setStack(const std::string &name, unsigned int amount = 1);

	protected:
		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

	private:
		Inventory &m_inventory;

		unsigned int m_x = 0;
		unsigned int m_y = 0;

		gk::Image m_image;
		Text m_text;

		Cube m_cube{10};
};

#endif // ITEMWIDGET_HPP_
