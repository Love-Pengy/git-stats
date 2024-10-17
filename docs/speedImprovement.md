# Notes for potential solutions to speed improvements

- FIRST UP FIGURE OUT THE PROFILER FULLY
- the second thing that should be looked at is the overall algo 

## Things that could potentially affect performance
- passing large memory structures around 
- calling non inline functions all the time
- using io all the time 

### General 
- get rid of some of the obs_log warnings/errors they're everywhere and giving some overhead 



**TODO**
```C 
    //tracked paths and untracked files should be allocated on the stack instead of the heap
    // max absolute file paths in linux are 256 characters
    struct gitData {
      char *trackedPaths[100];
      int numTrackedFiles;
      time_t trackedRepoMTimes[MAXNUMPATHS];
      int prevAddedValues_Tracked[MAXNUMPATHS];
      int prevDeletedValues_Tracked[MAXNUMPATHS];
      char *untrackedFiles[100];
      int numUntrackedFiles;
      time_t untrackedRepoMTimes[MAXNUMPATHS];
      int prevAddedValues_Untracked[MAXNUMPATHS];
      long previousUntrackedAdded;
      bool deletionEnabled;
      bool deletionSymbolEnabled;
      bool insertionEnabled;
      bool insertionSymbolEnabled;
      int delayAmount;
      long added;
      long deleted;
      char *overloadChar;
    };

```

### git_stats_update
------------------------------------------------------------
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
```
-------------------------------------------------------------
``` C
    // verify what gets returned on empty for obs_data_get_string likely don't have to check for both 
    // NOTE: IT JUST RETURNS AN EMPTY STRING **NOT NULL**
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



### git_stats_tick

``` C
  // Figure out why when ending the profile it tries to end the root 
  os_set_thread_name("gitStatsTickThread");  
  profile_start("git_stats_tick");
```


**Generally for this entire function it can most likely be done in a switch case with general statements for all**
- Ideas
    - make a switch case with a binary mapping for the cosmetic settings settings
        - info we have to consider
            - trackedPaths && numTrackedFiles (verifying that there has been nothing specified) 
            - insertionEnabled 
            - deletionEnabled
            - insertionSymbolEnabled
            - deletionSymbolEnabled
        - pros to this approach: 
            - we can only run source_update once without having to consider other context
            - potentially a speed increase with switch case  
            - readability  
    - offload each section to its own function
        - we could pass in all info into their own respective functions           
        - this potentially makes code significantly easier to read in terms of the tick function itself 
        - but this still requires some sort of optimizations within the new functions we create 
        - this potentially makes code significantly easier to read in terms of the tick function itself 
    - we just save the settings data within the gitData structure so we don't call the getter functions over and over  
        - all this would require is changing vars  
        - *NOTE* Be careful as both the tick and update functions require this and both can be running in seperate threads 
*Ensure that obs_source_update is not getting run multiple times as that's going to be a bottleneck we want it run as little as possbile*



### Functions That Need To Be Looked At
- Extract Unicode 
- ltoa
