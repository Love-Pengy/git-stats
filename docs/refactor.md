# Implementation Path For git-stats Refactor 

## INIT
+ First allocate the needed structs for both shared memory and the structs accessed by the parent 
+ start up text sources 
+ init the threads that will update the diff info and idle it 
    + one for untracked, and one for tracked
+ set structs to default 


## UPDATE

+ obtain all user settings 
+ apply settings to relevent structs 
+ update shared memory with updated directories to check for 

## TICK

+ check for minimum time elapsed (slow = 10, normal = 5, quick = 3)
+ grab values, parse through the settings to remove what isn't wanted
