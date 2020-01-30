/*
 * =====================================================================================
 *
 *       Filename:  Registry.cpp
 *
 *    Description:
 *
 *        Created:  23/06/2018 22:43:56
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include <gk/core/Exception.hpp>

#include "CraftingRecipe.hpp"
#include "SmeltingRecipe.hpp"
#include "Registry.hpp"

Registry *Registry::s_instance = nullptr;

Item &Registry::registerItem(const std::string &textureFilename, const std::string &id, const std::string &name) {
	u32 internalID = m_items.size();
	m_itemsID.emplace(id, internalID);
	m_items.emplace_back(internalID, textureFilename, id, name);
	return m_items.back();
}

const Block &Registry::getBlockFromStringID(const std::string &id) {
	if (id.empty()) return getBlock(0);
	auto it = m_blocksID.find(id);
	if (it == m_blocksID.end())
		throw EXCEPTION("Unknown block:", id);
	return getBlock(it->second);
}

const Item &Registry::getItemFromStringID(const std::string &id) {
	if (id.empty()) return getItem(0);
	auto it = m_itemsID.find(id);
	if (it == m_itemsID.end())
		throw EXCEPTION("Unknown item:", id);
	return getItem(it->second);
}

const Recipe *Registry::getRecipe(const Inventory &inventory) const {
	for (auto &recipe : m_recipes) {
		if (recipe->isMatching(inventory))
			return recipe.get();
	}
	return nullptr;
}

void Registry::serialize(sf::Packet &packet) {
	for (auto &it : m_items) {
		packet << u8(DataType::Item) << it.id() << it.name() << it.label()
			<< it.textureFilename() << it.isBlock() << it.isFuel()
			<< it.burnTime() << it.miningSpeed() << it.harvestCapability();
	}

	for (auto &it : m_blocks) {
		packet << u8(DataType::Block) << u32(it->id()) << it->name() << it->label() << u8(it->drawType())
			<< it->textureFilename() << it->hardness() << it->harvestRequirements() << it->getItemDrop();
	}

	for (auto &it : m_recipes) {
		packet << u8((it->type() == "craft") ? DataType::CraftingRecipe : DataType::SmeltingRecipe);
		it->serialize(packet);
	}
}

void Registry::deserialize(sf::Packet &packet) {
	u8 type;
	u32 id;
	std::string textureFilename, name, label;
	while (!packet.endOfPacket()) {
		packet >> type;
		if (type == u8(DataType::Block)) {
			u8 harvestRequirements, drawType;
			ItemStack itemDrop;
			float hardness;
			packet >> id >> name >> label >> drawType >> textureFilename >> hardness
				>> harvestRequirements >> itemDrop;

			auto &block = registerBlock<Block>(textureFilename, name, label);
			block.setDrawType(BlockDrawType(drawType));
			block.setHardness(hardness);
			block.setHarvestRequirements(harvestRequirements);
			block.setItemDrop(itemDrop.item().name(), itemDrop.amount());
		}
		else if (type == u8(DataType::Item)) {
			bool isFuel, isBlock;
			u8 harvestCapability;
			float miningSpeed;
			u16 burnTime;
			packet >> id >> name >> label >> textureFilename >> isBlock >> isFuel
				>> burnTime >> miningSpeed >> harvestCapability;

			auto &item = registerItem(textureFilename, name, label);
			item.setIsBlock(isBlock);
			item.setIsFuel(isFuel);
			item.setBurnTime(burnTime);
			item.setMiningSpeed(miningSpeed);
			item.setHarvestCapability(harvestCapability);
		}
		else if (type == u8(DataType::CraftingRecipe)) {
			registerRecipe<CraftingRecipe>()->deserialize(packet);
		}
		else if (type == u8(DataType::SmeltingRecipe)) {
			registerRecipe<SmeltingRecipe>()->deserialize(packet);
		}
	}
}

