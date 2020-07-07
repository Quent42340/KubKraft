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
#include "ClientCommandHandler.hpp"
#include "InventoryWidget.hpp"

void InventoryWidget::init(Inventory &inventory, u16 offset, u16 size) {
	m_inventory = &inventory;

	m_offset = offset;
	m_size = size > 0 ? size : inventory.width() * inventory.height();

	loadItemWidgets(m_offset, m_size);

	m_width = inventory.width() * 18;
	m_height = inventory.height() * 18;

	m_inventoryWidth = inventory.width();
	m_inventoryHeight = inventory.height();
}

void InventoryWidget::scroll(float scrolling) {
	u16 offset = m_offset + floor((m_inventory->height() - m_size / m_inventory->width()) * scrolling) * m_inventory->width();
	u16 size = m_size;

	if (offset + size > m_inventory->width() * m_inventory->height())
		size = m_inventory->width() * m_inventory->height() - offset;

	loadItemWidgets(offset, size);
}

void InventoryWidget::onEvent(const SDL_Event &event) {
	if (event.type == SDL_MOUSEMOTION) {
		m_currentItemWidget = nullptr;
		for (std::size_t i = 0 ; i < m_itemWidgets.size() ; ++i) {
			if (m_itemWidgets[i].isPointInWidget(event.motion.x, event.motion.y)) {
				m_currentItemWidget = &m_itemWidgets[i];

				m_selectedItemBackground.setPosition(1 + (i % m_inventoryWidth) * 18, 1 + (i / m_inventoryWidth) * 18, 0);
			}
		}
	}
}

void InventoryWidget::update() {
	bool hasChanged = false;

	for (auto &it : m_itemWidgets) {
		it.update();

		hasChanged = hasChanged || it.hasChanged();
		it.setChanged(false);
	}

	if (hasChanged)
		sendUpdatePacket();
}

bool InventoryWidget::sendItemStackToDest(const ItemWidget *itemStack, AbstractInventoryWidget *dest) {
	if (dest->doItemMatchFilter(itemStack->stack().item()) && dest->receiveItemStack(itemStack, this)) {
		if (dest != this)
			m_inventory->clearStack(itemStack->x(), itemStack->y());

		update();
		sendUpdatePacket();
		return true;
	}

	return false;
}

bool InventoryWidget::receiveItemStack(const ItemWidget *itemStack, AbstractInventoryWidget *src) {
	ItemStack stack = itemStack->stack();
	if (src == this)
		m_inventory->clearStack(itemStack->x(), itemStack->y());

	bool stackAdded = m_inventory->addStack(stack.item().stringID(), stack.amount(), m_offset, m_size);

	if (stackAdded)
		sendUpdatePacket();
	else if (src == this)
		m_inventory->setStack(itemStack->x(), itemStack->y(), stack.item().stringID(), stack.amount());

	return stackAdded;
}

void InventoryWidget::sendUpdatePacket() {
	if (m_inventory->inBlock()) {
		m_client.sendBlockInvUpdate(*m_inventory);
	}
	else {
		m_client.sendPlayerInvUpdate();
	}
}

void InventoryWidget::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	states.transform *= getTransform();

	for (std::size_t i = 0 ; i < m_itemWidgets.size() ; ++i) {
		target.draw(m_itemWidgets[i], states);
	}

	if (m_currentItemWidget)
		target.draw(m_selectedItemBackground, states);
}

void InventoryWidget::loadItemWidgets(u16 offset, u16 size) {
	m_itemWidgets.clear();

	for (u16 i = 0 ; i < size ; ++i) {
		m_itemWidgets.emplace_back(*m_inventory, (i + offset) % m_inventory->width(), (i + offset) / m_inventory->width(), this);

		ItemWidget &widget = m_itemWidgets.back();
		widget.update();
		widget.setPosition((i % m_inventory->width()) * 18, (i / m_inventory->width()) * 18, 0);
	}
}

