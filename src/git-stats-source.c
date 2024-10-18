
#include "./git-stats-source.h"

#include <errno.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdlib.h>
#include <util/threading.h>

#include "./git-diff-interface.h"
#include "support.h"

#define OVERLOAD_VAL 9999
#define MAX_OVERLOAD 4
#define DEFAULT_OVERLOAD_CHAR "."

// global for putting source in "max number" mode
static bool testMode = false;

// default font obj
static obs_data_t *defaultFont;

// global for the initial startup
static int INIT_RUN = 1;

// global for running update on startup
static int INIT_UPDATE = 1;

// global for instant update
static bool FORCE_UPDATE = false;

//TODO: get rid of these they aren't needed
static void git_stats_update(void *, obs_data_t *);
static void git_stats_get_defaults(obs_data_t *);
static obs_properties_t *git_stats_properties(void *);

struct gitStatsInfo {
	obs_source_t *insertionSource;
	// pointer to the text source
	// pointer to the deletion text source
	obs_source_t *deletionSource;
	// pointer to our source!!
	obs_source_t *gitSource;
	// length of the text source
	uint32_t cx;
	// height of the text source
	uint32_t cy;
	// time passed between the updates
	float time_passed;
	// the information that we get from our git stats thingy madonker
	struct gitData *data;
};

static const char *git_stats_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return (obs_module_text("Git Stats"));
}

// initializes the source
static void *git_stats_create(obs_data_t *settings, obs_source_t *source)
{
	struct gitStatsInfo *info = bzalloc(sizeof(struct gitStatsInfo));
	info->time_passed = 0;
	info->gitSource = source;

	const char *text_source_id = "text_ft2_source_v2";
	errno = 0;
	info->data = bzalloc(sizeof(struct gitData));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	info->data->trackedPaths = NULL;
	info->data->numTrackedFiles = 0;
	info->data->untrackedFiles = NULL;
	info->data->numUntrackedFiles = 0;
	info->data->previousUntrackedAdded = 0;
	info->data->added = 0;
	info->data->deleted = 0;
	info->data->insertionEnabled = true;
	info->data->deletionEnabled = true;
	info->data->insertionSymbolEnabled = true;
	info->data->deletionSymbolEnabled = true;
	errno = 0;
	// bmalloc 8 bytes for each unicode character
	info->data->overloadChar = bmalloc(16);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	info->data->overloadChar[0] = '\0';
	info->data->numUntrackedFiles = 0;
	info->data->untrackedFiles = NULL;

	defaultFont = obs_data_create();
	obs_data_set_default_string(defaultFont, "face", "DejaVu Sans Mono");
	obs_data_set_default_int(defaultFont, "size", 256);
	obs_data_set_default_int(defaultFont, "flags", 0);
	obs_data_set_default_string(defaultFont, "style", "");

	strncpy(info->data->overloadChar, DEFAULT_OVERLOAD_CHAR,
		strlen(DEFAULT_OVERLOAD_CHAR) + 1);

	info->insertionSource = obs_source_create(
		text_source_id, "insertionSource", settings, NULL);
	obs_source_add_active_child(info->gitSource, info->insertionSource);
	info->deletionSource =
		obs_source_create(text_source_id, "deletionSource", NULL, NULL);
	obs_source_add_active_child(info->gitSource, info->deletionSource);
	obs_source_set_hidden(info->insertionSource, true);
	obs_source_set_hidden(info->deletionSource, true);

	// ensure that defaults are set AFTER the creation has completed
	git_stats_get_defaults(settings);
	git_stats_update(info, settings);

	obs_log(LOG_INFO, "Source Created");

	return (info);
}

// bfree up the source and its data
static void git_stats_destroy(void *data)
{
	struct gitStatsInfo *info = data;

	obs_source_remove_active_child(info->gitSource, info->insertionSource);
	obs_source_remove_active_child(info->gitSource, info->deletionSource);

	obs_source_remove(info->insertionSource);
	obs_source_release(info->insertionSource);
	info->insertionSource = NULL;

	obs_source_remove(info->deletionSource);
	obs_source_release(info->deletionSource);
	info->deletionSource = NULL;

	bfree(info->data->overloadChar);
	info->data->overloadChar = NULL;

	for (int i = 0; i < info->data->numTrackedFiles; i++) {
		bfree(info->data->trackedPaths[i]);
		info->data->trackedPaths[i] = NULL;
	}
	for (int i = 0; i < info->data->numUntrackedFiles; i++) {
		bfree(info->data->untrackedFiles[i]);
		info->data->untrackedFiles[i] = NULL;
	}

	obs_data_release(defaultFont);

	if (info->data->untrackedFiles) {
		bfree(info->data->untrackedFiles);
		info->data->untrackedFiles = NULL;
	}
	if (info->data->trackedPaths) {
		bfree(info->data->trackedPaths);
		info->data->trackedPaths = NULL;
	}

	bfree(info->data);
	info->data = NULL;

	bfree(info);
	info = NULL;

	obs_log(LOG_INFO, "Source Destroyed");
}

// get the width needed for the source
static uint32_t git_stats_width(void *data)
{
	struct gitStatsInfo *info = data;
	if (info->data->deletionEnabled) {
		return (obs_source_get_width(info->deletionSource));
	}
	return (obs_source_get_width(info->insertionSource));
}

// get the height needed for the source
static uint32_t git_stats_height(void *data)
{
	struct gitStatsInfo *info = data;
	if (info->data->insertionEnabled) {
		return (obs_source_get_height(info->insertionSource));
	}
	return (obs_source_get_height(info->deletionSource));
}

// Settings that are loaded when the default button is hit in the properties
// window
static void git_stats_get_defaults(obs_data_t *settings)
{
	// repo settings
	obs_data_set_default_int(settings, "delay", 5);
	obs_data_set_default_string(settings, "overload_char", ".");
	obs_data_set_default_bool(settings, "untracked_files", false);

	// shared settings
	obs_data_set_default_bool(settings, "antialiasing", true);
	obs_data_set_default_bool(settings, "outline", false);
	obs_data_set_default_bool(settings, "drop_shadow", false);

	// deletion opts
	obs_data_set_default_int(settings, "deletion_color1", 0xFF0000FF);
	obs_data_set_default_int(settings, "deletion_color2", 0xFF0000FF);
	obs_data_set_default_bool(settings, "deletion_symbol", true);

	// insertion opts
	obs_data_set_default_int(settings, "color1", 0xFF00FF00);
	obs_data_set_default_int(settings, "color2", 0xFF00FF00);
	obs_data_set_default_bool(settings, "insertion_symbol", true);

	// make DejaVu Sans Mono the default because sans serif is not mono and
	// doesn't require nerd fonts installed
	obs_data_set_default_obj(settings, "font", defaultFont);

	// group settings
	obs_data_set_default_bool(settings, "insertion_properties", true);
	obs_data_set_default_bool(settings, "deletion_properties", true);
}

// update the settings data in our source
static void git_stats_update(void *data, obs_data_t *settings)
{
	struct gitStatsInfo *info = data;

	// copy settings from dummy property to the deletion text source
	obs_data_t *insertionFont = obs_data_get_obj(settings, "font");
	obs_data_t *gsSettings = obs_source_get_settings(info->gitSource);
	obs_data_t *isSettings = obs_source_get_settings(info->insertionSource);
	obs_data_t *dsSettings = obs_source_get_settings(info->deletionSource);

	obs_data_set_obj(dsSettings, "font", insertionFont);
	obs_data_release(insertionFont);

	obs_data_set_bool(dsSettings, "antialiasing",
			  obs_data_get_bool(settings, "antialiasing"));
	obs_data_set_int(dsSettings, "color1",
			 obs_data_get_int(settings, "deletion_color1"));
	obs_data_set_int(dsSettings, "color2",
			 obs_data_get_int(settings, "deletion_color2"));
	obs_data_set_bool(dsSettings, "outline",
			  obs_data_get_bool(settings, "outline"));
	obs_data_set_bool(dsSettings, "drop_shadow",
			  obs_data_get_bool(settings, "drop_shadow"));

	char *unicode =
		extractUnicode(obs_data_get_string(settings, "overload_char"));
	if (unicode) {
		strcpy(info->data->overloadChar, unicode);
		bfree(unicode);
	} else {
		strncpy(info->data->overloadChar, DEFAULT_OVERLOAD_CHAR,
			strlen(DEFAULT_OVERLOAD_CHAR) + 1);
	}

	info->data->insertionEnabled =
		obs_data_get_bool(settings, "insertion_properties");
	info->data->insertionSymbolEnabled =
		obs_data_get_bool(settings, "insertion_symbol");
	info->data->deletionEnabled =
		obs_data_get_bool(settings, "deletion_properties");
	info->data->deletionSymbolEnabled =
		obs_data_get_bool(settings, "deletion_symbol");

	obs_data_array_t *dirArray =
		obs_data_get_array(gsSettings, "single_repos");

	//free data if nothing specified before changing numtrackedfiles to 0
	if (info->data->numTrackedFiles > 0) {
		for (int i = 0; i < info->data->numTrackedFiles; i++) {
			bfree(info->data->trackedPaths[i]);
			info->data->trackedPaths[i] = NULL;
		}
	}
	if (info->data->numUntrackedFiles > 0) {
		for (int i = 0; i < info->data->numUntrackedFiles; i++) {
			bfree(info->data->untrackedFiles[i]);
			info->data->untrackedFiles[i] = NULL;
		}
	}
	if (!info->data->trackedPaths) {
		errno = 0;
		info->data->trackedPaths =
			bmalloc(sizeof(char *) * MAXNUMPATHS);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
		}
		for (int i = 0; i < MAXNUMPATHS; i++) {
			info->data->trackedPaths[i] = NULL;
			info->data->prevAddedValues_Tracked[i] = 0;
			info->data->prevDeletedValues_Tracked[i] = 0;
			info->data->trackedRepoMTimes[i] = 0;
			info->data->prevAddedValues_Untracked[i] = 0;
			info->data->untrackedRepoMTimes[i] = 0;
		}
	}
	if (!info->data->untrackedFiles) {
		errno = 0;
		info->data->untrackedFiles =
			bmalloc(sizeof(char *) * MAXNUMPATHS);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
		}
		for (int i = 0; i < MAXNUMPATHS; i++) {
			info->data->untrackedFiles[i] = NULL;
		}
	}
	info->data->numTrackedFiles = 0;
	info->data->numUntrackedFiles = 0;

	//single repos
	for (size_t i = 0; i < obs_data_array_count(dirArray); i++) {
		obs_data_t *currItem = obs_data_array_item(dirArray, i);
		const char *currVal = obs_data_get_string(currItem, "value");
		errno = 0;
		info->data->trackedPaths[i] =
			bmalloc(sizeof(char) * strlen(currVal) + 1);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
			obs_data_release(currItem);
			info->data->trackedPaths[i] = NULL;
			info->data->trackedRepoMTimes[i] = 0;
			info->data->numTrackedFiles++;
			continue;
		}
		if (checkPath((char *)currVal)) {
			strncpy(info->data->trackedPaths[i], currVal,
				strlen(currVal) + 1);
			info->data->trackedRepoMTimes[i] =
				getModifiedTime(info->data->trackedPaths[i]);
			info->data->numTrackedFiles++;
		}
		obs_data_release(currItem);
	}
	obs_data_array_release(dirArray);
	// directory of repos
	if (strcmp(obs_data_get_string(settings, "repositories_directory"),
		   "")) {
		addGitRepoDir(info->data,
			      (char *)obs_data_get_string(
				      settings, "repositories_directory"));
	}
  
	if (obs_data_get_bool(settings, "untracked_files")) {
		createUntrackedFiles(info->data);
	}
	info->data->delayAmount = obs_data_get_int(settings, "delay");
	info->data->added = 0;
	info->data->deleted = 0;

	if (gsSettings) {
		obs_data_release(gsSettings);
	}
	if (isSettings) {
		obs_data_release(isSettings);
	}
	if (dsSettings) {
		obs_data_release(dsSettings);
	}
	FORCE_UPDATE = true;
}

// render out the source
static void git_stats_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct gitStatsInfo *info = data;
	obs_source_video_render(info->insertionSource);
	obs_source_video_render(info->deletionSource);
}

// update relevant real time data for the source (called each frame with the
// time elapsed passed in)
static void git_stats_tick(void *data, float seconds)
{
	//os_set_thread_name("gitStatsTickThread");
	profile_start("git_stats_tick");
	struct gitStatsInfo *info = data;
	if (!obs_source_showing(info->gitSource)) {
		return;
	}
	info->time_passed += seconds;
	if (info->time_passed > info->data->delayAmount || INIT_RUN ||
	    FORCE_UPDATE) {
		if (checkUntrackedFiles(info->data) &&
		    info->data->numUntrackedFiles) {
			obs_data_t *currSettings =
				obs_source_get_settings(info->gitSource);
			obs_source_update(info->gitSource, currSettings);
			obs_data_release(currSettings);
		}
		obs_data_t *isSettings =
			obs_source_get_settings(info->insertionSource);
		obs_data_t *dsSettings =
			obs_source_get_settings(info->deletionSource);
		obs_data_t *gsSettings =
			obs_source_get_settings(info->gitSource);
		INIT_RUN &= 0;
		FORCE_UPDATE = false;
		info->time_passed = 0;
		if (testMode) {
			int numOverload = MAX_OVERLOAD;
			errno = 0;
			char *overloadString = bmalloc(
				sizeof(char) * (MB_CUR_MAX * numOverload));
			if (errno) {
				obs_log(LOG_ERROR, "%s (%d): %s", __FILE__,
					__LINE__, strerror(errno));
				if (isSettings) {
					obs_data_release(isSettings);
				}
				if (dsSettings) {
					obs_data_release(dsSettings);
				}
				if (gsSettings) {
					obs_data_release(gsSettings);
				}
				return;
			}
			overloadString[0] = ' ';
			overloadString[1] = '\0';
			for (volatile int i = 1; i < numOverload + 1; i++) {
				strcat(overloadString,
				       info->data->overloadChar);
			}
			char buffer[256] = "";
			char *overloadValueString = ltoa(OVERLOAD_VAL);
			snprintf(buffer,
				 strlen(overloadValueString) +
					 strlen(overloadString) + 3,
				 "%s\n+%s", overloadString,
				 overloadValueString);
			obs_data_set_string(isSettings, "text", buffer);
			obs_source_update(info->insertionSource, isSettings);

			char spaces[7] = "";
			int deletionSize = strlen(overloadValueString) + 2;
			for (int i = 0; i < deletionSize; i++) {
				spaces[i] = ' ';
				spaces[i + 1] = '\0';
			}
			snprintf(buffer,
				 strlen(overloadString) + (strlen(spaces) * 2) +
					 strlen(overloadValueString) + 3,
				 "%s%s\n%s-%s", spaces, overloadString, spaces,
				 overloadValueString);
			obs_data_set_string(dsSettings, "text", buffer);
			obs_source_update(info->deletionSource, dsSettings);
			bfree(overloadValueString);
			bfree(overloadString);
			if (gsSettings) {
				obs_data_release(gsSettings);
			}
			if (isSettings) {
				obs_data_release(isSettings);
			}
			if (dsSettings) {
				obs_data_release(dsSettings);
			}
			return;
		}
		if (info->data->trackedPaths == NULL ||
		    !info->data->numTrackedFiles) {
			if (info->data->insertionEnabled) {
				if (info->data->insertionSymbolEnabled) {
					obs_data_set_string(isSettings, "text",
							    "\n   +0");
					obs_source_update(info->insertionSource,
							  isSettings);
				} else {
					obs_data_set_string(isSettings, "text",
							    "\n    0");
					obs_source_update(info->insertionSource,
							  isSettings);
				}
			} else {
				obs_data_set_string(isSettings, "text", "    ");
				obs_source_update(info->insertionSource,
						  isSettings);
			}
			if (info->data->deletionEnabled) {
				if (info->data->deletionSymbolEnabled) {
					obs_data_set_string(dsSettings, "text",
							    "\n      -0   ");
					obs_source_update(info->deletionSource,
							  dsSettings);
				} else {
					obs_data_set_string(dsSettings, "text",
							    "\n       0   ");
					obs_source_update(info->deletionSource,
							  dsSettings);
				}
			} else {
				obs_data_set_string(dsSettings, "text",
						    "         ");
				obs_source_update(info->deletionSource,
						  dsSettings);
			}
			if (gsSettings) {
				obs_data_release(gsSettings);
			}
			if (isSettings) {
				obs_data_release(isSettings);
			}
			if (dsSettings) {
				obs_data_release(dsSettings);
			}
			return;
		} else {
			info->data->deleted = 0;
			info->data->added = 0;
			updateTrackedFiles(info->data, INIT_UPDATE);
			if (!checkUntrackedFileLock(info->data)) {
				info->data->previousUntrackedAdded =
					updateUntrackedFiles(info->data,
							     INIT_UPDATE);
			} else {
				info->data->added +=
					info->data->previousUntrackedAdded;
			}
			INIT_UPDATE &= 0;
		}
		int spaceCheck = (info->data->insertionEnabled << 1) |
				 info->data->deletionEnabled;
		if (info->data->insertionEnabled) {
			long value = info->data->added;
			int numOverload = value / OVERLOAD_VAL;
			value = value % OVERLOAD_VAL;
			numOverload > MAX_OVERLOAD ? numOverload = MAX_OVERLOAD
						   : numOverload;
			char *overloadString = NULL;
			if (!numOverload) {
				errno = 0;
				overloadString = bmalloc(sizeof(char) * 2);

				if (errno) {
					if (gsSettings) {
						obs_data_release(gsSettings);
					}
					if (isSettings) {
						obs_data_release(isSettings);
					}
					if (dsSettings) {
						obs_data_release(dsSettings);
					}
					obs_log(LOG_ERROR, "%s (%d): %s",
						__FILE__, __LINE__,
						strerror(errno));

					return;
				}
				overloadString[0] = '\0';
			} else {
				errno = 0;
				overloadString = bmalloc(
					sizeof(char) *
					(strlen(info->data->overloadChar) *
					 numOverload));
				if (errno) {
					obs_log(LOG_ERROR, "%s (%d): %s",
						__FILE__, __LINE__,
						strerror(errno));
					if (gsSettings) {
						obs_data_release(gsSettings);
					}
					if (isSettings) {
						obs_data_release(isSettings);
					}
					if (dsSettings) {
						obs_data_release(dsSettings);
					}
					return;
				}
				overloadString[1] = '\0';
				overloadString[0] = ' ';
			}
			for (volatile int i = 1; i < numOverload + 1; i++) {
				strcat(overloadString,
				       info->data->overloadChar);
			}

			char outputBuffer[100] = "";
			char *valueString = ltoa(value);
			char insertionSpaces[4] = "";
			if (spaceCheck == 0b11) {
				for (size_t i = 0; i < 4 - strlen(valueString);
				     i++) {
					insertionSpaces[i] = ' ';
					insertionSpaces[i + 1] = '\0';
				}
			} else {
				strncpy(insertionSpaces, "   ", 4);
			}
			if (info->data->insertionSymbolEnabled) {
				snprintf(outputBuffer,
					 strlen(overloadString) +
						 strlen(valueString) +
						 (strlen(insertionSpaces) * 2) +
						 3,
					 "%s%s\n%s+%s", insertionSpaces,
					 overloadString, insertionSpaces,
					 valueString);
				obs_data_set_string(isSettings, "text",
						    outputBuffer);
			} else {
				snprintf(outputBuffer,
					 strlen(overloadString) +
						 strlen(valueString) +
						 (strlen(insertionSpaces) * 2) +
						 3,
					 "%s%s\n %s%s", insertionSpaces,
					 overloadString, insertionSpaces,
					 valueString);
				obs_data_set_string(isSettings, "text",
						    outputBuffer);
			}
			bfree(valueString);
			bfree(overloadString);
		} else {
			char outputBuffer[100] = " ";
			obs_data_set_string(isSettings, "text", outputBuffer);
		}
		char outputBuffer[100] = "\0";
		if (info->data->deletionEnabled) {
			long deletionValue = info->data->deleted;
			int numOverload = deletionValue / OVERLOAD_VAL;
			numOverload > MAX_OVERLOAD ? numOverload = MAX_OVERLOAD
						   : numOverload;
			deletionValue = deletionValue % OVERLOAD_VAL;
			char *overloadString = NULL;
			if (!numOverload) {
				errno = 0;
				overloadString = bmalloc(sizeof(char) * 2);
				if (errno) {
					obs_log(LOG_ERROR, "%s (%d): %s",
						__FILE__, __LINE__,
						strerror(errno));
					if (isSettings) {
						obs_data_release(isSettings);
					}
					if (dsSettings) {
						obs_data_release(dsSettings);
					}
					if (gsSettings) {
						obs_data_release(gsSettings);
					}
					return;
				}
				overloadString[0] = '\0';
			} else {
				errno = 0;
				overloadString = bmalloc(
					sizeof(char) *
					(strlen(info->data->overloadChar) *
					 numOverload));
				if (errno) {
					obs_log(LOG_ERROR, "%s (%d): %s",
						__FILE__, __LINE__,
						strerror(errno));
					if (isSettings) {
						obs_data_release(isSettings);
					}
					if (dsSettings) {
						obs_data_release(dsSettings);
					}
					if (gsSettings) {
						obs_data_release(gsSettings);
					}
					return;
				}
				overloadString[1] = '\0';
				overloadString[0] = ' ';
			}
			for (int i = 1; i < numOverload + 1; i++) {
				strcat(overloadString,
				       info->data->overloadChar);
			}
			char deletionSpaces[7] = "";
			char *deletionValueString = ltoa(deletionValue);
			if (info->data->insertionEnabled) {
				for (size_t i = 0;
				     i < (4 - strlen(deletionValueString));
				     i++) {
					deletionSpaces[i] = ' ';
					deletionSpaces[i + 1] = '\0';
				}
			} else {
				strncpy(deletionSpaces, "   ", 4);
			}
			if (info->data->deletionSymbolEnabled) {
				if (spaceCheck == 0b11) {
					snprintf(
						outputBuffer,
						strlen(overloadString) +
							(strlen(deletionSpaces) *
							 2) +
							strlen(deletionValueString) +
							3 + 12,
						"%s%s%s\n%s-%s%s", "      ",
						overloadString, deletionSpaces,
						"      ", deletionValueString,
						deletionSpaces);
				} else {
					snprintf(
						outputBuffer,
						strlen(overloadString) +
							(strlen(deletionSpaces) *
							 2) +
							strlen(deletionValueString) +
							3,
						"%s%s\n%s-%s%s", deletionSpaces,
						overloadString, deletionSpaces,
						deletionValueString,
						deletionSpaces);
				}
				obs_data_set_string(dsSettings, "text",
						    outputBuffer);
			} else {
				if (spaceCheck == 0b11) {
					snprintf(
						outputBuffer,
						strlen(overloadString) +
							(strlen(deletionSpaces) *
							 2) +
							strlen(deletionValueString) +
							3 + 12,
						"%s%s%s\n%s %s%s", "      ",
						overloadString, deletionSpaces,
						"      ", deletionValueString,
						deletionSpaces);
				} else {
					snprintf(
						outputBuffer,
						strlen(overloadString) +
							(strlen(deletionSpaces) *
							 2) +
							strlen(deletionValueString) +
							3 + 12,
						"%s%s\n %s%s", overloadString,
						deletionSpaces,
						deletionValueString,
						deletionSpaces);
				}

				obs_data_set_string(dsSettings, "text",
						    outputBuffer);
			}
			bfree(deletionValueString);
			bfree(overloadString);
		} else {
			char outputBuff[100] = " ";
			obs_data_set_string(dsSettings, "text", outputBuff);
		}

		obs_source_update(info->deletionSource, dsSettings);
		obs_source_update(info->insertionSource, isSettings);
		if (gsSettings) {
			obs_data_release(gsSettings);
		}
		if (isSettings) {
			obs_data_release(isSettings);
		}
		if (dsSettings) {
			obs_data_release(dsSettings);
		}
	}
	profile_end("git_stats_tick");
	profile_reenable_thread();
}

// callback for the test_button property
static bool toggleTestCallback(obs_properties_t *properties,
			       obs_property_t *buttonProps, void *data)
{
	UNUSED_PARAMETER(properties);
	UNUSED_PARAMETER(buttonProps);
	UNUSED_PARAMETER(data);
	testMode ^= 1;
	FORCE_UPDATE = true;
	return (true);
}

// properties that are generated in the ui of the source
static obs_properties_t *git_stats_properties(void *unused)
{
	struct gitStatsInfo *info = unused;
	UNUSED_PARAMETER(unused);
	obs_data_t *isSettings = obs_source_get_settings(info->insertionSource);

	obs_properties_t *props = obs_properties_create();

	obs_properties_t *repo_props = obs_properties_create();

	obs_properties_add_path(repo_props, "repositories_directory",
				"Directory Holding Repositories",
				OBS_PATH_DIRECTORY, NULL, NULL);

	obs_properties_add_editable_list(repo_props, "single_repos",
					 "Single Repositories",
					 OBS_EDITABLE_LIST_TYPE_FILES, NULL,
					 NULL);

	obs_properties_add_int(repo_props, "delay", "Delay Between Updates", 1,
			       INT_MAX, 1);

	obs_properties_add_text(repo_props, "overload_char",
				"Character Shown For Overload",
				OBS_TEXT_DEFAULT);

	obs_properties_add_bool(repo_props, "untracked_files",
				"Account For Untracked Files");

	obs_properties_add_button(repo_props, "test_button", "Test Max Size",
				  toggleTestCallback);

	obs_properties_add_group(props, "repo_properties",
				 "Repository Settings", OBS_GROUP_NORMAL,
				 repo_props);

	///////////////

	obs_properties_t *shared_props =
		obs_source_properties(info->insertionSource);
	obs_properties_remove_by_name(shared_props, "text_file");
	obs_properties_remove_by_name(shared_props, "from_file");
	obs_properties_remove_by_name(shared_props, "log_mode");
	obs_properties_remove_by_name(shared_props, "log_lines");
	obs_properties_remove_by_name(shared_props, "word_wrap");
	obs_properties_remove_by_name(shared_props, "text");
	obs_properties_remove_by_name(shared_props, "custom_width");
	obs_properties_remove_by_name(shared_props, "color1");
	obs_properties_remove_by_name(shared_props, "color2");
	obs_properties_add_group(props, "shared_properties", "Shared Settings",
				 OBS_GROUP_NORMAL, shared_props);

	///////////////

	obs_properties_t *text1_props =
		obs_source_properties(info->insertionSource);
	obs_properties_remove_by_name(text1_props, "font");
	obs_properties_remove_by_name(text1_props, "text_file");
	obs_properties_remove_by_name(text1_props, "from_file");
	obs_properties_remove_by_name(text1_props, "log_mode");
	obs_properties_remove_by_name(text1_props, "log_lines");
	obs_properties_remove_by_name(text1_props, "word_wrap");
	obs_properties_remove_by_name(text1_props, "text");
	obs_properties_remove_by_name(text1_props, "custom_width");
	obs_properties_remove_by_name(text1_props, "drop_shadow");
	obs_properties_remove_by_name(text1_props, "outline");
	obs_properties_remove_by_name(text1_props, "antialiasing");
	obs_properties_add_bool(text1_props, "insertion_symbol", "+ Symbol");
	obs_data_set_default_int(isSettings, "color1", 0xFF00FF00);
	obs_data_set_default_int(isSettings, "color1", 0xFF00FF00);
	obs_properties_add_group(props, "insertion_properties",
				 "Insertion Settings", OBS_GROUP_CHECKABLE,
				 text1_props);

	//////////////

	obs_properties_t *text2_props = obs_properties_create();
	obs_properties_add_color_alpha(text2_props, "deletion_color1",
				       "Color1");
	obs_properties_add_color_alpha(text2_props, "deletion_color2",
				       "Color2");
	obs_properties_add_bool(text2_props, "deletion_symbol", "- Symbol");

	obs_properties_add_group(props, "deletion_properties",
				 "Deletion Settings", OBS_GROUP_CHECKABLE,
				 text2_props);

	obs_data_release(isSettings);
	return props;
}

// clang-format off
    struct obs_source_info git_stats_source = {
        .id           = "git-stats",
        .type         = OBS_SOURCE_TYPE_INPUT,
        .output_flags = OBS_SOURCE_VIDEO,
        .get_name     = git_stats_name,
        .create       = git_stats_create,
        .destroy      = git_stats_destroy,
        .update       = git_stats_update,
        .video_render = git_stats_render,
        .get_width    = git_stats_width,
        .get_height   = git_stats_height,
        .video_tick = git_stats_tick, 
        .get_properties = git_stats_properties, 
        .icon_type = OBS_ICON_TYPE_TEXT, 
    };
// clang-format on
