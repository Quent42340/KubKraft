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
#include "Biome.hpp"
#include "LuaMod.hpp"
#include "PlacementEntry.hpp"
#include "Registry.hpp"
#include "Tree.hpp"

void LuaMod::commit() {
	while (!m_defs.empty()) {
		auto &it = m_defs.front();

		switch (it.first) {
			case DefinitionType::Block:           m_blockLoader.loadBlock(it.second);            break;
			case DefinitionType::Item:            m_itemLoader.loadItem(it.second);              break;
			case DefinitionType::CraftingRecipe:  m_recipeLoader.loadCraftingRecipe(it.second);  break;
			case DefinitionType::SmeltingRecipe:  m_recipeLoader.loadSmeltingRecipe(it.second);  break;
			case DefinitionType::Sky:             m_skyLoader.loadSky(it.second);                break;
			case DefinitionType::Tree:            m_biomeLoader.loadTree(it.second);             break;
			case DefinitionType::Biome:           m_biomeLoader.loadBiome(it.second);            break;
			case DefinitionType::Dimension:       m_dimensionLoader.loadDimension(it.second);    break;
			case DefinitionType::Entity:          m_entityLoader.loadEntity(it.second);          break;
			default: break;
		}

		m_defs.pop();
	}
}

#define DEF_FUNC(type) [](LuaMod &self, const sol::table &table) { \
	self.m_defs.emplace(type, table); \
}

void LuaMod::initUsertype(sol::state &lua) {
	lua.new_usertype<LuaMod>("LuaMod",
		"id",              &LuaMod::id,
		"path",            &LuaMod::path,

		"spawn_entity",    &LuaMod::spawnEntity,

		"block",           DEF_FUNC(DefinitionType::Block),
		"item",            DEF_FUNC(DefinitionType::Item),
		"crafting_recipe", DEF_FUNC(DefinitionType::CraftingRecipe),
		"smelting_recipe", DEF_FUNC(DefinitionType::SmeltingRecipe),
		"sky",             DEF_FUNC(DefinitionType::Sky),
		"tree",            DEF_FUNC(DefinitionType::Tree),
		"biome",           DEF_FUNC(DefinitionType::Biome),
		"dimension",       DEF_FUNC(DefinitionType::Dimension),
		"entity",          DEF_FUNC(DefinitionType::Entity)
	);
}

void LuaMod::spawnEntity(const std::string &entityID, const sol::table &table) {
	m_entityLoader.spawnEntity(entityID, table);
}

