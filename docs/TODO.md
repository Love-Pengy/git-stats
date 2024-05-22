## FEATURES

- specify multiple repos
- set refresh rate
- untracked files
- plus lines && minus lines
  - enable
  - change font
  - font size
  - color
  - number only
- text will always be centered
  - or this could also be a more creative way like every 1k put an indicator somewhere
- ability to convey to the user useful error outputs
  - for invalid paths coming from the config

## OVERALL LOGIC

- get the output of the diff command & store it  
- add all of the collective +'s and -'s together to get the sum that should be outputted
- parse through it to get the data in a segmented form
- depending on the settings output it in a specific format

## NOTES

- DO NOT WORRY ABOUT WINDOWS THIS IS FOR YOU ATM

## ASSUMPTIONS

- when grabbing input from the obs ui/user the output of that will be an array of strings
- if we have the string "insertions" within the output of git diff then we have a full valid output
- we are expecting the end directory to have the / at the end
- no one in their right mind would have either 4 mil insertions or deletions

## TIMELINE

- create the utility using purely the command line
- fix builing of the plugin
- slap that bad boi in obs form
- $$profit$$

## TODO

- ~~get git diff info for file specified by me~~
- ~~add '/' to the end of all paths if they do not already have it~~
- ~~take a list of paths and get the git diff info from all of them~~
- ~~take out insertions/deletions from entire shortstat line~~
  - ~~cut up the line using the commas as delimiters~~
    - ~~using that cut up section grab the long from it~~
- ~~add all of those together~~
  - ~~use a long for this~~

- ~~REWRITE THE ENTIRITY OF THE BASE~~
  - ~~interface with the freetype2 text source itself~~
  - ~~get the source to showup correctly~~
  - ~~get the rendering working correctly~~
  - ~~get the updating fixed~~
- ~~create module for hash table implementation~~
  - ~~create submodule for hash table input~~

- implement git diff into the plugin
- reformat git-diff implementation to be a module and not a run file itself
- add the ability to get untracked files
  - use this command to get the file path of the untracked files ```git ls-files --others --exclude-standard```
  - given the output of this command count the number of lines in that file at the current moment and store it in a hash table
  - add this to the count of insertions/deletions (depending on the original value)
  - when getting these untracked files ignore executables

## CURRENT PATH

- ~~test updateValue in the hash table~~
- NOTE: SPLIT THIS UP INTO EACH PART AND CALL THEM FROM THE SOURCE FILE
  - as you've seen it gets very messy if doing everything in the source file

- ~~decide if I want hash table or hash map (leaning towards map)~~
- ~~put two hash tables or maps inside of the obs source data structure to represent both tracked and untracked files~~
  - ~~replaced this with one hashmap and an array of paths because tracked files only need the paths~~
- ~~update hashtable and hashmap to be taking untracked files instead of boarding passes~~
- ~~update the CMakeLists file to include the hashmap library~~
- ~~get rid of the following properties for the text source:~~
  - ~~text~~
  - ~~read from file~~
  - ~~chat log mode~~
  - ~~chat log lines~~
  - ~~text file~~

- create handlers for both untracked and tracked files
  - for tracked you just need to get the info from the git diff output
  - for untracked you need to actually go through and manage those

- when calling the update function for the source, when gettings the repos you must delimit the string by newlines in order to get the paths
- we interface with both or one of these handlers depending on what the use set the settings to
  - ex. if the user does not want untracked files then we would never update the untracked portion of the data
- reorder the properties so that the text properties are at the bottom
- figure out a way to do popups or something else for the user so they know when errors have happened
