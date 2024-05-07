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
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// some sort of driving structure
// struct blah{
//  directories
//  current count
//      removed
//      added
// }
//

static void git_stats_update(void) {
    FILE* fp;
    char output[1000];
    // do this for all items within the structure
    fp = popen(
        "/usr/bin/git diff --shortstat /home/Bee/Projects/git-stats", "r");

    if (fp == NULL) {
        printf("FP WAS NULL\n");
        exit(1);
    }

    while (fgets(output, sizeof(output), fp)) {
        printf("%s\n", output);
    }
    pclose(fp);
}

bool obs_module_load(void) {
    obs_log(
        LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
    return true;
}

void obs_module_unload(void) { obs_log(LOG_INFO, "plugin unloaded"); }
