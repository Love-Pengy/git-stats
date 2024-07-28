#include <obs-module.h>

#include "support.h"

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("git-stats", "en_US")

extern struct obs_source_info git_stats_source;

bool obs_module_load(void)
{
	obs_register_source(&git_stats_source);
	obs_log(LOG_INFO, "PLUGIN LOADED SUCCESSFULLY");
	return (true);
}
