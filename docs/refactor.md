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
    + make this a time change between last completed update so it's based on performance of IO operation
+ grab values, parse through the settings to remove what isn't wanted

### OVERALL THREAD PATH

+ create two threads, one for tracked one 
+ those threads will stay dormant until a set of tracked/untracked strings are present
    + consider making a struct for this (holds an array of strings, the amount of strings in the array, the status of the thread (updated, updating), the kill signal)
+ if the thread fails then through the signal portion of the struct send a failure  
+ if destroy function was hit send close signal through struct
 
### CHANGES REQUIRED 

+ implement threads 
+ remove non needed memory
+ categorize source files better (main file should not be like 2000 lines)
+ refactor update and tick to be as lightweight as possible the threads should be doing the hard work


### TODO

+ research faster ways to get git diff info
    + maybe git diff alternatives?
+ see if it's possible to just keep a shell open so I eliminate the opening and closing of it
*this is going to take very long*
+ **move to pthreads** 
+ update ui and settings to specify slow, normal, and quick update times 
+ go back and rework everything to be readable and make sense  



