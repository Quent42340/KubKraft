/*
 * =====================================================================================
 *
 *       Filename:  CraftingRecipe.hpp
 *
 *    Description:
 *
 *        Created:  24/06/2018 06:38:11
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef CRAFTINGRECIPE_HPP_
#define CRAFTINGRECIPE_HPP_

#include <array>
#include <map>

#include "Recipe.hpp"

class CraftingRecipe : public Recipe {
	public:
		CraftingRecipe() : Recipe("craft") {}
		CraftingRecipe(const std::vector<std::string> &pattern, const std::map<char, std::vector<std::string>> &keys, const ItemStack &result, bool isShapeless = false);

		void serialize(sf::Packet &packet) const override;
		void deserialize(sf::Packet &packet) override;

		bool isMatching(const Inventory &inventory) const override;

	private:
		bool checkMatch(const Inventory &inventory, int offsetX, int offsetY) const;

		std::vector<std::string> m_pattern;
		std::map<char, std::vector<std::string>> m_keys;

		bool m_isShapeless = false;
};

#endif // CRAFTINGRECIPE_HPP_
