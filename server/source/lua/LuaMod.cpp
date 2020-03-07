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
#include <map>

#include "Biome.hpp"
#include "CraftingRecipe.hpp"
#include "LuaMod.hpp"
#include "PlacementEntry.hpp"
#include "Registry.hpp"
#include "ServerBlock.hpp"
#include "SmeltingRecipe.hpp"
#include "Tree.hpp"

void LuaMod::registerBlock(const sol::table &table) {
	TilesDef tiles;
	tiles.loadFromLuaTable(table);

	std::string stringID = m_id + ":" + table["id"].get<std::string>();
	std::string label = table["name"].get<std::string>();
	sol::function onBlockActivated = table["on_block_activated"];
	sol::function onTick = table["on_tick"];

	ServerBlock &block = Registry::getInstance().registerBlock<ServerBlock>(tiles, stringID, label);
	block.setHarvestRequirements(table["harvest_requirements"].get_or(0));
	block.setHardness(table["hardness"].get_or(1.0f));
	block.setOpaque(table["is_opaque"].get_or(true));
	block.setLightSource(table["is_light_source"].get_or(false));
	block.setOnBlockActivated(onBlockActivated);
	block.setOnTick(onTick);
	block.setOnBlockPlaced(table["on_block_placed"]);
	block.setRotatable(table["is_rotatable"].get_or(false));

	sol::optional<sol::table> boundingBox = table["bounding_box"];
	if (boundingBox != sol::nullopt) {
		block.setBoundingBox(gk::FloatBox{
			boundingBox.value().get<float>(1),
			boundingBox.value().get<float>(2),
			boundingBox.value().get<float>(3),
			boundingBox.value().get<float>(4),
			boundingBox.value().get<float>(5),
			boundingBox.value().get<float>(6),
		});
	}

	// FIXME: Use string instead
	sol::optional<BlockDrawType> drawType = table["draw_type"];
	if (drawType != sol::nullopt) {
		block.setDrawType(drawType.value());
	}

	sol::optional<sol::table> itemDrop = table["item_drop"];
	if (itemDrop != sol::nullopt) {
		std::string dropID = itemDrop.value()["id"];
		u16 dropAmount = itemDrop.value()["amount"];
		block.setItemDrop(dropID, dropAmount);
	}

	sol::optional<sol::table> colorMultiplier = table["color_multiplier"];
	if (colorMultiplier != sol::nullopt) {
		block.setColorMultiplier(gk::Color{
			colorMultiplier.value().get<u8>(1),
			colorMultiplier.value().get<u8>(2),
			colorMultiplier.value().get<u8>(3),
			colorMultiplier.value().get<u8>(4)
		});
	}

	Registry::getInstance().registerItem(block.tiles(), stringID, label).setIsBlock(true);
}

void LuaMod::registerItem(const sol::table &table) {
	TilesDef tiles;
	tiles.loadFromLuaTable(table);

	std::string stringID = m_id + ":" + table["id"].get<std::string>();
	std::string label = table["name"].get<std::string>();

	Item &item = Registry::getInstance().registerItem(tiles, stringID, label);
	item.setIsFuel(table["is_fuel"].get_or(false));
	item.setBurnTime(table["burn_time"].get_or(0));
	item.setHarvestCapability(table["harvest_capability"].get_or(0));
	item.setMiningSpeed(table["mining_speed"].get_or(1));
}

void LuaMod::registerCraftingRecipe(const sol::table &table) {
	sol::table resultTable = table["result"];
	sol::table patternTable = table["pattern"];
	sol::table keysTable = table["keys"];

	ItemStack result = {
		resultTable["id"].get<std::string>(),
		resultTable["amount"].get<u16>()
	};

	std::vector<std::string> pattern;
	for (auto &it : patternTable)
		pattern.emplace_back(it.second.as<std::string>());

	std::map<char, std::vector<std::string>> keys;
	for (auto &it : keysTable) {
		keys.emplace(it.first.as<char>(), std::vector<std::string>{it.second.as<std::string>()});
	}

	Registry::getInstance().registerRecipe<CraftingRecipe>(pattern, keys, result);
}

void LuaMod::registerSmeltingRecipe(const sol::table &table) {
	sol::table inputTable = table["input"];
	sol::table outputTable = table["output"];

	ItemStack input = {
		inputTable["id"].get<std::string>(),
		inputTable["amount"].get<u16>()
	};

	ItemStack output = {
		outputTable["id"].get<std::string>(),
		outputTable["amount"].get<u16>()
	};

	Registry::getInstance().registerRecipe<SmeltingRecipe>(input, output);
}

void LuaMod::registerTree(const sol::table &table) {
	std::string stringID = m_id + ":" + table["id"].get<std::string>();
	std::string label = table["name"].get<std::string>();

	Tree &tree = Registry::getInstance().registerTree(stringID, label);
	tree.setLogBlockID(Registry::getInstance().getBlockFromStringID(table["log_block"]).id());
	tree.setLeavesBlockID(Registry::getInstance().getBlockFromStringID(table["leaves_block"]).id());
}

void LuaMod::registerBiome(const sol::table &table) {
	std::string stringID = m_id + ":" + table["id"].get<std::string>();
	std::string label = table["name"].get<std::string>();

	Biome &biome = Registry::getInstance().registerBiome(stringID, label);

	// TODO eventually a WorldType could have a list of biome parameter names in order,
	// and we could use those as the ordered keys.
	// Currently hardcoding "temperature" and "precipitation" to get something functional.
	biome.addParameter(table["params"]["temperature"]);
	biome.addParameter(table["params"]["precipitation"]);

	biome.setTopBlockID(Registry::getInstance().getBlockFromStringID(table["top_block"]).id());
	biome.setGroundBlockID(Registry::getInstance().getBlockFromStringID(table["ground_block"]).id());
	biome.setDeepBlockID(Registry::getInstance().getBlockFromStringID(table["deep_block"]).id());
	biome.setBeachBlockID(Registry::getInstance().getBlockFromStringID(table["beach_block"]).id());
	biome.setLiquidBlockID(Registry::getInstance().getBlockFromStringID(table["liquid_block"]).id());

	sol::optional<sol::table> treeDefinitions = table["trees"];
	if (treeDefinitions != sol::nullopt) {
		for (const auto &it : treeDefinitions.value()) {
			sol::table treeDefinition = it.second.as<sol::table>();

			PlacementEntry::Tree &treeEntry = biome.addTree();
			treeEntry.treeID = Registry::getInstance().getTreeFromStringID(treeDefinition["type"]).id();
			treeEntry.probability = treeDefinition["probability"];
		}
	}

	sol::optional<sol::table> floraDefinitions = table["flora"];
	if (floraDefinitions != sol::nullopt) {
		for (const auto &it : floraDefinitions.value()) {
			sol::table floraDefinition = it.second.as<sol::table>();

			PlacementEntry::Flora &floraEntry = biome.addFlora();
			floraEntry.blockID = Registry::getInstance().getBlockFromStringID(floraDefinition["block"]).id();
			floraEntry.spawnsOnBlockID = Registry::getInstance().getBlockFromStringID(floraDefinition["spawns_on"]).id();
			floraEntry.probability = floraDefinition["probability"];
		}
	}

	sol::optional<sol::table> oreDefinitions = table["ore"];
	if (oreDefinitions != sol::nullopt) {
		for (const auto &it : oreDefinitions.value()) {
			sol::table oreDefinition = it.second.as<sol::table>();

			PlacementEntry::Ore &oreEntry = biome.addOre();
			oreEntry.blockID = Registry::getInstance().getBlockFromStringID(oreDefinition["block"]).id();
			oreEntry.probability = oreDefinition["probability"];
			oreEntry.size = oreDefinition["size"];
		}
	}
}

void LuaMod::initUsertype(sol::state &lua) {
	lua.new_usertype<LuaMod>("LuaMod",
		sol::constructors<LuaMod(std::string)>(),
		"id",              &LuaMod::id,
		"block",           &LuaMod::registerBlock,
		"item",            &LuaMod::registerItem,
		"crafting_recipe", &LuaMod::registerCraftingRecipe,
		"smelting_recipe", &LuaMod::registerSmeltingRecipe,
		"tree",            &LuaMod::registerTree,
		"biome",           &LuaMod::registerBiome
	);
}

