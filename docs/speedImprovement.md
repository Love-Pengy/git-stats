# Notes for potential solutions to speed improvements

- FIRST UP FIGURE OUT THE PROFILER FULLY
- the second thing that should be looked at is the overall algo 

### git_stats_update
``` C
    // we could potentially just write the extracted unicode to the overloadChar directly
    // also check to see if we need to do a check for "" and NULL (see which one gets output from get_data)
	if (strcmp(obs_data_get_string(settings, "overload_char"), "") &&
	    obs_data_get_string(settings, "overload_char")) {
		char *unicode = extractUnicode(
			obs_data_get_string(settings, "overload_char"));
		if (unicode) {
			strcpy(info->data->overloadChar, unicode);
			bfree(unicode);
		} else {
			strncpy(info->data->overloadChar, DEFAULT_OVERLOAD_CHAR,
				strlen(DEFAULT_OVERLOAD_CHAR) + 1);
		}
	} else {
		strncpy(info->data->overloadChar, DEFAULT_OVERLOAD_CHAR,
			strlen(DEFAULT_OVERLOAD_CHAR) + 1);
	}
``` C
    // verify what gets returned on empty for obs_data_get_string likely don't have to check for both 
	if (strcmp(obs_data_get_string(settings, "repositories_directory"),
		   "") &&
	    (obs_data_get_string(settings, "repositories_directory") != NULL)) {
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
```
------------------------------------------------------------------------

### git_stats_tick

``` C
  // Figure out why when ending the profile it tries to end the root 
  os_set_thread_name("gitStatsTickThread");  
  profile_start("git_stats_tick");
```

**Generally for this entire function it can most likely be done in a switch case with general statements for all**
*Ensure that obs_source_update is not getting run multiple times as that's going to be a bottleneck we want it run as little as possbile*


## Things that could potentially affect performance
- passing large memory structures around 
- calling non inline functions all the time
- using io all the time 
