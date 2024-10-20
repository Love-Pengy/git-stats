
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

		int modeMapping = !info->data->numTrackedFiles |
				  (testMode << 1);
		int spaceCheck = (info->data->insertionEnabled << 1) |
				 info->data->deletionEnabled;

		INIT_UPDATE &= 0;
		char insertionValue_String[OVERLOAD_VAL] = "0";
		char deletionValue_String[OVERLOAD_VAL] = "0";
		char insertionOverloadString[(OVERLOAD_VAL * 4) + 1] = " ";
		char deletionOverloadString[(OVERLOAD_VAL * 4) + 1] = " ";
		int numOverload = 0;
    long value = 0;

		// default = test mode AKA 10 and 11
		switch (modeMapping): 
      case 00:
			// we know we have data to calaculate for
      //TODO: THIS NEEDS TO CALCULATE BOTH THE DELETION AND INSERTION SOURCE DATA
			info->data->deleted = 0;
			info->data->added = 0;
      updateTrackedFiles(info->data, INIT_UPDATE);
      if (!checkUntrackedFileLock(info->data)) {
        info->data->previousUntrackedAdded =
          updateUntrackedFiles(info->data, INIT_UPDATE);
      } else {
        info->data->added += info->data->previousUntrackedAdded;
      }
      //----------------------------------------------
      //                  INSERTION                 //
      //----------------------------------------------                                            
      value = info->data->added;
      numOverload = value / OVERLOAD_VAL;
      value = value % OVERLOAD_VAL;
      numOverload > MAX_OVERLOAD ? numOverload = MAX_OVERLOAD
               : numOverload;
      for (volatile int i = 0; i < numOverload; i++) {
        strcat(insertionOverloadString, info->data->overloadChar);
      }
      insertionValue_String = ltoa(value);
      //----------------------------------------------
      //                  DELETION                  //
      //----------------------------------------------                                            
      value = info->data->deleted;
			numOverload = value / OVERLOAD_VAL;
			numOverload > MAX_OVERLOAD ? numOverload = MAX_OVERLOAD
						   : numOverload;
			value = value % OVERLOAD_VAL;
      for (volatile int i = 0; i < numOverload; i++) {
        strcat(deletionOverloadString, info->data->overloadChar);
      }
      deletionValue_String = ltoa(value);
      break;
	case 01:
		// Non test mode, no data AKA 0's
		break;
	default:
		int numOverload = MAX_OVERLOAD;
		char overloadString[(MAX_OVERLOAD * 4)] = "";
		overloadString[0] = ' ';
		overloadString[1] = '\0';
		for (volatile int i = 0; i < numOverload; i++) {
			strcat(overloadString, info->data->overloadChar);
		}
		// overload (4 bytes per char)
		// two symbols (2 chars)
		// a newline (1 char)
		// a space
		char buffer[(4 * OVERLOAD_VAL * 2) + (OVERLOAD_VAL * 2) + 4] =
			"";
		char *overloadValueString = ltoa(OVERLOAD_VAL);
		snprintf(buffer,
			 strlen(overloadValueString) + strlen(overloadString) +
				 3,
			 "%s\n+%s", overloadString, overloadValueString);
		obs_data_set_string(isSettings, "text", buffer);

		// one for space, one for symbol
		char spaces[OVERLOAD_VAL + 2] = "";
		for (int i = 0; i < (OVERLOAD_VAL + 2); i++) {
			spaces[i] = ' ';
			spaces[i + 1] = '\0';
		}
		snprintf(buffer,
			 strlen(overloadString) + (strlen(spaces) * 2) +
				 strlen(overloadValueString) + 3,
			 "%s%s\n%s-%s", spaces, overloadString, spaces,
			 overloadValueString);
		obs_data_set_string(dsSettings, "text", buffer);

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





  int settingsMap = 0;
  settingsMap = (info->data->insertionEnabled) |
          ((info->data->insertionSymbolEnabled) << 1);
  settingsMap |= ((info->data->deletionEnabled) << 2) |
           ((info->data->deletionSymbolEnabled) << 3);
    /* NOTE: This calc should be dependent on the settings map 
      char insertionSpaces[4] = "";
      char deletionSpaces[7] = "";
      if (spaceCheck == 0b11) {
        for (size_t i = 0; i < 4 - strlen(valueString); i++) {
          insertionSpaces[i] = ' ';
          insertionSpaces[i + 1] = '\0';
        }
        else
        {
          strncpy(insertionSpaces, "    ", 4);
        }
      }
  */
  if (info->data->trackedPaths == NULL ||
      !info->data->numTrackedFiles) {
    //NOTE: WE ARE SAVING THE SOURCE UPDATING TO THE END OF TICK
    switch (settingsMap): 
      // 0000 0010 1000 1010 all default 
      case 0001:
      //insertion enabled
    case 0011:
      //insertion eneabled w/ symbol
    case 0100:
      //deletion enabled
    case 0101:
      //deletion enabled insertion eneabled (no symbols)
    case 0110:
      // deletion enabled no symbol
    case 0111:
      // deletion enabled no symbol, insertion enabled with symbol
    case 1001:
      // insertion enabled
    case 1011:
      // insertion enabled with symbol
    case 1100:
      // deletion enabled with symbol
    case 1101:
      //deletion eneabled with symbol, insertion enabled
    case 1110:
      //deletion eneabled with symbol
    case 1111:
      // all enabled
    default:
      // none enabled
      obs_data_set_string(isSettings, "text",
              "         ");
      break;

    profile_end("git_stats_tick");
    profile_reenable_thread();
		}
