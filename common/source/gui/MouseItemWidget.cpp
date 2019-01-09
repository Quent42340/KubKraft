/*
 * =====================================================================================
 *
 *       Filename:  MouseItemWidget.cpp
 *
 *    Description:
 *
 *        Created:  23/06/2018 00:51:15
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include "MouseItemWidget.hpp"

MouseItemWidget::MouseItemWidget(Widget *parent) : ItemWidget(m_inventory, 0, 0, parent) {
	m_tooltipBackground.setColor(gk::Color{255, 255, 255, 240});
	m_tooltipBackground.setPosition(20, 17, 0);

	m_tooltipText.setPosition(26, 24, 0);

	m_tooltipInfoText.setPosition(26, 35, 0);
	m_tooltipInfoText.setColor({180, 180, 180});
}

void MouseItemWidget::onEvent(const SDL_Event &event) {
	if (event.type == SDL_MOUSEMOTION) {
		updatePosition(event.motion.x, event.motion.y);
	}

	else if (event.type == SDL_MOUSEBUTTONDOWN) {
		updatePosition(event.button.x, event.button.y);
	}
}

void MouseItemWidget::update(const ItemWidget *currentItemWidget) {
	if (currentItemWidget) {
		m_currentItemWidget = (currentItemWidget->stack().item().id()) ? currentItemWidget : nullptr;
		m_tooltipText.setText(currentItemWidget->stack().item().label() + " [" + std::to_string(currentItemWidget->stack().item().id()) + "]");

		if (currentItemWidget->stack().item().isFuel())
			m_tooltipInfoText.setText("Burn time: " + std::to_string(currentItemWidget->stack().item().burnTime()) + " ticks");
		else
			m_tooltipInfoText.setText("");
	}
	else {
		m_currentItemWidget = nullptr;
	}
}

void MouseItemWidget::swapItems(ItemWidget &widget, bool isReadOnly) {
	std::string widgetItemName = widget.stack().item().name();
	u32 widgetItemAmount = widget.stack().amount();

	if (!isReadOnly || stack().item().id() == 0 || stack().item().name() == widgetItemName) {
		if (stack().item().name() != widgetItemName) {
			widget.setStack(stack().item().name(), stack().amount());
			setStack(widgetItemName, widgetItemAmount);
		}
		else if (!isReadOnly) {
			widget.setStack(widgetItemName, widgetItemAmount + stack().amount());
			setStack("", 0);
		}
		else {
			setStack(stack().item().name(), stack().amount() + widgetItemAmount);
			widget.setStack("", 0);
		}
	}
}

void MouseItemWidget::putItem(ItemWidget &widget) {
	std::string widgetItemName = widget.stack().item().name();
	u32 widgetItemID = widget.stack().item().id();
	u32 widgetItemAmount = widget.stack().amount();

	if (!widgetItemID && stack().item().id()) {
		widget.setStack(stack().item().name(), 1);
		setStack(stack().amount() > 1 ? stack().item().name() : "", stack().amount() - 1);
	}
	else if (widgetItemID && widgetItemID == stack().item().id()) {
		widget.setStack(stack().item().name(), widgetItemAmount + 1);
		setStack(stack().amount() > 1 ? stack().item().name() : "", stack().amount() - 1);
	}
	else if (stack().item().id() == 0) {
		setStack(widgetItemName, ceil(widgetItemAmount / 2.0));
		widget.setStack(widgetItemAmount > 1 ? widgetItemName : "", widgetItemAmount / 2);
	}
}

void MouseItemWidget::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	ItemWidget::draw(target, states);

	states.transform *= getTransform();

	if (m_currentItemWidget) {
		target.draw(m_tooltipBackground, states);
		target.draw(m_tooltipText, states);
		target.draw(m_tooltipInfoText, states);
	}
}

void MouseItemWidget::updatePosition(float x, float y) {
	x -= m_parent->getPosition().x + 10 * m_parent->getScale().x;
	y -= m_parent->getPosition().y + 10 * m_parent->getScale().y;

	setPosition(x / m_parent->getScale().x, y / m_parent->getScale().y, 0);
}

