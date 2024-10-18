
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
    
    int modeMapping = (testMode) | ((!info->data->numTrackedFiles) << 1); 
    switch (modeMapping): 
      case 00: 
        // we know we have data to calaculate for 
      case 01: 
        // Non test mode, no data AKA 0's
      case 10: 
        // test mode
      case 11: 
        // test mode
     
    int settingsMap = 0;
    settingsMap = (info->data->insertionEnabled) | ((info->data->insertionSymbolEnabled) << 1);      
    settingsMap |= ((info->data->deletionEnabled) << 2) | ((info->data->deletionSymbolEnabled) << 3);  

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
          obs_data_set_string(isSettings, "text", "         ");
          break;
    
	profile_end("git_stats_tick");
	profile_reenable_thread();
}
