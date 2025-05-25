#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("git-stats", "en_US")

extern struct obs_source_info git_stats_source;

MODULE_EXPORT const char *obs_module_description(void)
{
	return ("Show Git Diff Information For Selected Repositories");
}

bool obs_module_load(void)
{
	obs_register_source(&git_stats_source);
	return (true);
}
