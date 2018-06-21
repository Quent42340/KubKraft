/*
 * =====================================================================================
 *
 *       Filename:  InventoryWidget.hpp
 *
 *    Description:
 *
 *        Created:  21/06/2018 01:08:40
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef INVENTORYWIDGET_HPP_
#define INVENTORYWIDGET_HPP_

#include <vector>

#include "Inventory.hpp"
#include "ItemWidget.hpp"
#include "Transformable.hpp"

class InventoryWidget : public IDrawable, public Transformable {
	public:
		void update(const Inventory &inventory);

	private:
		void draw(RenderTarget &target, RenderStates states) const override;

		std::vector<ItemWidget> m_itemWidgets;
};

#endif // INVENTORYWIDGET_HPP_
