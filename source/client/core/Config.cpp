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
#include <fstream>

#include <gk/core/Debug.hpp>
#include <gk/core/Filesystem.hpp>

#include <sol.hpp>

#include "Config.hpp"

// Gameplay
bool Config::isFlyModeEnabled = false;
bool Config::isNoClipEnabled = false;

// Interface
bool Config::isBlockInfoWidgetEnabled = true;
bool Config::isFpsCounterEnabled = true;
bool Config::isHotbarVisible = true;
bool Config::isCrosshairVisible = true;

// Graphics
u16 Config::renderDistance = 8;
bool Config::isTorchSmoothLightingEnabled = true;
bool Config::isSunSmoothLightingEnabled = true;
bool Config::isAmbientOcclusionEnabled = false;
bool Config::isWireframeModeEnabled = false;
bool Config::isFullscreenModeEnabled = false;
bool Config::isVerticalSyncEnabled = true;
float Config::cameraFOV = 70.0f;
u16 Config::screenWidth = 1600;
u16 Config::screenHeight = 1050;
u8 Config::guiScale = 3;
u8 Config::mipmapLevels = 0;
float Config::aoStrength = 1.0f;

// Input
u8 Config::mouseSensitivity = 8;

void Config::loadConfigFromFile(const char *filename) {
	if (gk::Filesystem::fileExists(filename)) {
		sol::state lua;

		try {
			lua.safe_script_file(filename);

			isFlyModeEnabled = lua["isFlyModeEnabled"].get_or(isFlyModeEnabled);
			isNoClipEnabled = lua["isNoClipEnabled"].get_or(isNoClipEnabled);

			isBlockInfoWidgetEnabled = lua["isBlockInfoWidgetEnabled"].get_or(isBlockInfoWidgetEnabled);
			isFpsCounterEnabled = lua["isFpsCounterEnabled"].get_or(isFpsCounterEnabled);
			isHotbarVisible = lua["isHotbarVisible"].get_or(isHotbarVisible);
			isCrosshairVisible = lua["isCrosshairVisible"].get_or(isCrosshairVisible);

			renderDistance = lua["renderDistance"].get_or(renderDistance);
			isTorchSmoothLightingEnabled = lua["isTorchSmoothLightingEnabled"].get_or(isTorchSmoothLightingEnabled);
			isSunSmoothLightingEnabled = lua["isSunSmoothLightingEnabled"].get_or(isSunSmoothLightingEnabled);
			isAmbientOcclusionEnabled = lua["isAmbientOcclusionEnabled"].get_or(isAmbientOcclusionEnabled);
			isWireframeModeEnabled = lua["isWireframeModeEnabled"].get_or(isWireframeModeEnabled);
			isFullscreenModeEnabled = lua["isFullscreenModeEnabled"].get_or(isFullscreenModeEnabled);
			isVerticalSyncEnabled = lua["isVerticalSyncEnabled"].get_or(isVerticalSyncEnabled);
			cameraFOV = lua["cameraFOV"].get_or(cameraFOV);
			screenWidth = lua["screenWidth"].get_or(screenWidth);
			screenHeight = lua["screenHeight"].get_or(screenHeight);
			guiScale = lua["guiScale"].get_or(guiScale);
			mipmapLevels = lua["mipmapLevels"].get_or(mipmapLevels);
			aoStrength = lua["aoStrength"].get_or(aoStrength);

			mouseSensitivity = lua["mouseSensitivity"].get_or(mouseSensitivity);

			gkInfo() << "Config file loaded successfully";
		}
		catch (sol::error &e) {
			gkError() << e.what();
		}
	}
}

void Config::saveConfigToFile(const char *filename) {
	std::ofstream file{filename, std::ofstream::out | std::ofstream::trunc};
	file << "isFlyModeEnabled = " << (isFlyModeEnabled ? "true" : "false") << std::endl;
	file << "isNoClipEnabled = " << (isNoClipEnabled ? "true" : "false") << std::endl;
	file << std::endl;
	file << "isBlockInfoWidgetEnabled = " << (isBlockInfoWidgetEnabled ? "true" : "false") << std::endl;
	file << "isFpsCounterEnabled = " << (isFpsCounterEnabled ? "true" : "false") << std::endl;
	file << "isHotbarVisible = " << (isHotbarVisible ? "true" : "false") << std::endl;
	file << "isCrosshairVisible = " << (isCrosshairVisible ? "true" : "false") << std::endl;
	file << std::endl;
	file << "renderDistance = " << renderDistance << std::endl;
	file << "isTorchSmoothLightingEnabled = " << (isTorchSmoothLightingEnabled ? "true" : "false") << std::endl;
	file << "isSunSmoothLightingEnabled = " << (isSunSmoothLightingEnabled ? "true" : "false") << std::endl;
	file << "isAmbientOcclusionEnabled = " << (isAmbientOcclusionEnabled ? "true" : "false") << std::endl;
	file << "isWireframeModeEnabled = " << (isWireframeModeEnabled ? "true" : "false") << std::endl;
	file << "isFullscreenModeEnabled = " << (isFullscreenModeEnabled ? "true" : "false") << std::endl;
	file << "isVerticalSyncEnabled = " << (isVerticalSyncEnabled ? "true" : "false") << std::endl;
	file << "cameraFOV = " << cameraFOV << std::endl;
	file << "screenWidth = " << screenWidth << std::endl;
	file << "screenHeight = " << screenHeight << std::endl;
	file << "guiScale = " << (u16)guiScale << std::endl;
	file << "mipmapLevels = " << (u16)mipmapLevels << std::endl;
	file << "aoStrength = " << aoStrength << std::endl;
	file << std::endl;
	file << "mouseSensitivity = " << (u16)mouseSensitivity << std::endl;
}

