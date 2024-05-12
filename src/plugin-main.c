/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-source.h>
#include <plugin-support.h>
#include <stdbool.h>
#include <util/platform.h>

#include "find-font.h"
#include "ft2build.h"
#include "globals.h"

#define DEBUG 1

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

FT_Library ft2_lib;
bool plugin_initialized = false;

bool obs_module_load(void) {
    char* config_dir = obs_module_config_path(NULL);
    if (config_dir) {
        os_mkdirs(config_dir);
        bfree(config_dir);
    }

    obs_log(
        LOG_INFO, "[GIT STATS] plugin loaded successfully (version %s)",
        PLUGIN_VERSION);
    obs_log(LOG_INFO, "TEST: %d", DEBUG);
    if (DEBUG) obs_log(LOG_INFO, "[GIT STATS] Attempting To Load Source V1");
    obs_register_source(&freetype2_source_info_v1);
    if (DEBUG) obs_log(LOG_INFO, "[GIT STATS] Attempting To Load Source V2");
    obs_register_source(&freetype2_source_info_v2);
    return true;
}

void obs_module_unload(void) {
    if (DEBUG) obs_log(LOG_INFO, "[GIT STATS] Unloading Plugin");
    if (plugin_initialized) {
        free_os_font_list();
        FT_Done_FreeType(ft2_lib);
    }

    obs_log(LOG_INFO, "[GIT STATS] plugin unloaded");
}
