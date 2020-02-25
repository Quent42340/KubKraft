--
-- =====================================================================================
--
--  OpenMiner
--
--  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
--  Copyright (C) 2019-2020 the OpenMiner contributors (see CONTRIBUTORS.md)
--
--  This file is part of OpenMiner.
--
--  OpenMiner is free software; you can redistribute it and/or
--  modify it under the terms of the GNU Lesser General Public
--  License as published by the Free Software Foundation; either
--  version 2.1 of the License, or (at your option) any later version.
--
--  OpenMiner is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--  Lesser General Public License for more details.
--
--  You should have received a copy of the GNU Lesser General Public License
--  along with OpenMiner; if not, write to the Free Software Foundation, Inc.,
--  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
--
-- =====================================================================================
--

-- Wooden Axe
mod:crafting_recipe {
	result = {
		id = "default:wooden_axe",
		amount = 1
	},

	pattern = {
		"##",
		"#|",
		" |"
	},

	keys = {
		['#'] = "default:planks",
		['|'] = "default:stick",
	}
}

-- Wooden Hoe
mod:crafting_recipe {
	result = {
		id = "default:wooden_hoe",
		amount = 1
	},

	pattern = {
		"##",
		" |",
		" |"
	},

	keys = {
		['#'] = "default:planks",
		['|'] = "default:stick",
	}
}

-- Wooden Pickaxe
mod:crafting_recipe {
	result = {
		id = "default:wooden_pickaxe",
		amount = 1
	},

	pattern = {
		"###",
		" | ",
		" | "
	},

	keys = {
		['#'] = "default:planks",
		['|'] = "default:stick",
	}
}

-- Wooden Shovel
mod:crafting_recipe {
	result = {
		id = "default:wooden_shovel",
		amount = 1
	},

	pattern = {
		"#",
		"|",
		"|"
	},

	keys = {
		['#'] = "default:planks",
		['|'] = "default:stick",
	}
}

-- Wooden Sword
mod:crafting_recipe {
	result = {
		id = "default:wooden_sword",
		amount = 1
	},

	pattern = {
		"#",
		"#",
		"|"
	},

	keys = {
		['#'] = "default:planks",
		['|'] = "default:stick",
	}
}

-- Stone Axe
mod:crafting_recipe {
	result = {
		id = "default:stone_axe",
		amount = 1
	},

	pattern = {
		"##",
		"#|",
		" |"
	},

	keys = {
		['#'] = "default:cobblestone",
		['|'] = "default:stick",
	}
}

-- Stone Hoe
mod:crafting_recipe {
	result = {
		id = "default:stone_hoe",
		amount = 1
	},

	pattern = {
		"##",
		" |",
		" |"
	},

	keys = {
		['#'] = "default:cobblestone",
		['|'] = "default:stick",
	}
}

-- Stone Pickaxe
mod:crafting_recipe {
	result = {
		id = "default:stone_pickaxe",
		amount = 1
	},

	pattern = {
		"###",
		" | ",
		" | "
	},

	keys = {
		['#'] = "default:cobblestone",
		['|'] = "default:stick",
	}
}

-- Stone Shovel
mod:crafting_recipe {
	result = {
		id = "default:stone_shovel",
		amount = 1
	},

	pattern = {
		"#",
		"|",
		"|"
	},

	keys = {
		['#'] = "default:cobblestone",
		['|'] = "default:stick",
	}
}

-- Stone Sword
mod:crafting_recipe {
	result = {
		id = "default:stone_sword",
		amount = 1
	},

	pattern = {
		"#",
		"#",
		"|"
	},

	keys = {
		['#'] = "default:cobblestone",
		['|'] = "default:stick",
	}
}

-- Stick
mod:crafting_recipe {
	result = {
		id = "default:stick",
		amount = 4
	},

	pattern = {
		'#',
		'#'
	},

	keys = {['#'] = "default:planks"}
}

-- Planks
mod:crafting_recipe {
	result = {
		id = "default:planks",
		amount = 4
	},
	pattern = {"#"},
	keys = {["#"] = "default:wood"}
}

-- Workbench
mod:crafting_recipe {
	result = {
		id = "default:workbench",
		amount = 1
	},
	pattern = {
		"##",
		"##"
	},
	keys = {["#"] = "default:planks"}
}

-- Furnace
mod:crafting_recipe {
	result = {
		id = "default:furnace",
		amount = 1
	},

	pattern = {
		"###",
		"# #",
		"###"
	},

	keys = {["#"] = "default:cobblestone"}
}

-- Glowstone
mod:crafting_recipe {
	result = {
		id = "default:glowstone",
		amount = 1
	},

	pattern = {"#"},

	keys = {['#'] = "default:cobblestone"},
}

-- Iron Ingot
mod:smelting_recipe {
	input = {id = "default:iron_ore", amount = 1},
	output = {id = "default:iron_ingot", amount = 1}
}

-- Stone
mod:smelting_recipe {
	input = {id = "default:cobblestone", amount = 1},
	output = {id = "default:stone", amount = 1}
}

-- Glass
mod:smelting_recipe {
	input = {id = "default:sand", amount = 1},
	output = {id = "default:glass", amount = 1}
}

-- Charcoal
mod:smelting_recipe {
	input = {id = "default:wood", amount = 1},
	output = {id = "default:charcoal", amount = 1}
}

-- Brick
mod:smelting_recipe {
	input = {id = "default:clay_ball", amount = 1},
	output = {id = "default:brick", amount = 1}
}

-- Bricks
mod:crafting_recipe {
	result = {
		id = "default:bricks",
		amount = 1
	},
	pattern = {
		"##",
		"##"
	},
	keys = {["#"] = "default:brick"}
}

-- Stone Bricks
mod:crafting_recipe {
	result = {
		id = "default:stone_bricks",
		amount = 1
	},
	pattern = {
		"##",
		"##"
	},
	keys = {["#"] = "default:stone"}
}
