#!/usr/bin/python3

import subprocess
import configargparse
import signal
import json
from dataclasses import dataclass, fields
import random
import shutil
import os
import time
import re

@dataclass
class testRecap:
    min: float 
    max: float 
    nineth: float 
    median: float 
    successes: int
    failures: int

# header for determining test log
LOG_HEADER = "[git-stats] [TEST]"

# header for an added line in random file 
ADD_HEADER = "[GIT-STATS-TEST-SCRIPTS]"

# terminal color codes 
SUCCESS = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
RESET = '\033[0m'

# https://stackoverflow.com/a/13790289
def tail(f, lines=1, _buffer=4098):
    """Tail a file and get X lines from the end"""
    # place holder for the lines found
    lines_found = []

    # block counter will be multiplied by buffer
    # to get the block size from the end
    block_counter = -1

    # loop until we find X lines
    while len(lines_found) < lines:
        try:
            f.seek(block_counter * _buffer, os.SEEK_END)
        except IOError: # either file is too small, or too many lines requested
            f.seek(0)
            lines_found = f.readlines()
            break

        lines_found = f.readlines()

        block_counter -= 1

    return lines_found[-lines:]

# get newest path in dir
def newest(path: str) -> str:
    """Get newest file in directory

    Keyword arguments: 
    path: path to directory 
    """

    files = os.listdir(path)
    paths = [os.path.join(path, basename) for basename in files]
    return max(paths, key=os.path.getctime)

def getInsertions(diff: str) -> int:
    """Get Number of Insertions from `git diff --shortstat` string
    
    Keyword arguments:
    diff: string output from `git diff --shortstat`
    """
    
    split = diff.split(",")   

    # see if insertion exists in any of the split strings 
    insertionIndex = [index for index, content in enumerate(split) if 
        "insertion" in content]
    insertionsIndex = [index for index, content in enumerate(split) if 
        "insertions" in content]
    if(insertionsIndex):
        return(int(re.findall(r"\d+", split[insertionsIndex[0]])[0]))
    elif(insertionIndex):
        return(int(re.findall(r"\d+", split[insertionIndex[0]])[0]))
    else: 
        return(0)

def getDeletions(diff) -> int:
    """Get Number of Deletions from `git diff --shortstat` string
    
    Keyword arguments:
    diff: string output from `git diff --shortstat`
    """

    split = diff.split(",")   

    # see if insertion exists in any of the split strings 
    deletionIndex = [index for index, content in enumerate(split) 
        if "deletion" in content]
    deletionsIndex = [index for index, content in enumerate(split) 
        if "deletions" in content]
    if(deletionsIndex):
        return(int(re.findall(r"\d+", split[deletionsIndex[0]])[0]))
    elif(deletionIndex):
        return(int(re.findall(r"\d+", split[deletionIndex[0]])[0]))
    else: 
        return(0)

# returns tuple (insertions,deletions)
def getDiffOutput(test_scene_path: str, 
                  addedPaths: dict, 
                  backupDir: str) -> tuple:
    """Get output of git diff for all repos 
        specified in git-stats plugin config

        args: 
            test_scene_path: path to scenes dir
            addedPaths: dictionary that holds the place where the backup 
                was added to and where it originated from  
            backupDir: path in which backup for changes files should go 
    """
    
    # Obtains git-stats source information 
    with open(test_scene_path) as f: 
        config_sources = json.load(f)["sources"]
        for item in config_sources:
            if(item["name"] == "Git Stats"):
                settings = item["settings"] 
    
    # Handle Untracked
    if(("untracked_files" in settings) and (settings["untracked_files"])): 
        if("repositories_directory" in settings): 
            dirs = next(os.walk(settings["repositories_directory"]))[1]
            for dir in dirs:
                # Put all untracked file in index with "intent to add" feature
                args = [shutil.which("git"), "-C", 
                        f"{settings["repositories_directory"]}/{dir}", 
                        "add", "-N", "."] 
                out = subprocess.run(args) 

        if("single_repos" in settings): 
            repoSpecs = settings["single_repos"]
            for spec in repoSpecs:
                args = [shutil.which("git"), "-C", spec["value"], "add", 
                        "-N", "."] 
                out = subprocess.run(args) 
            
    insertions = 0;
    deletions = 0;
    
    if(("repositories_directory" in settings) 
        and ("single_repos" in settings)):
        # choose between repo dir and single repos for adding lines 
        chosen = random.randint(1,2)
    elif("repositories_directory" in settings):
        chosen = 1
    else: 
        chosen = 2

    # add = 1, del = 2
    mode = random.randint(1,2)
    currPath = None

    if("repositories_directory" in settings):
        dirs = next(os.walk(settings["repositories_directory"]))[1]
        for dir in dirs:
            args = [shutil.which("git"), "-C", 
                    f"{settings["repositories_directory"]}/{dir}", 
                    "diff", "--shortstat"] 
            out = subprocess.run(args, capture_output=True, text=True) 
            insertions += getInsertions(out.stdout)
            deletions += getDeletions(out.stdout)

        if(chosen == 1): 
            currPath = f"{settings["repositories_directory"]}/{random.choice(dirs)}" 
            currPath += "/"+random.choice([file for file in os.listdir(currPath) if 
                                          os.path.isfile(os.path.join(currPath, file))])
    
    if("single_repos" in settings):
        repoSpecs = settings["single_repos"]
        for spec in repoSpecs:
            args = [shutil.which("git"), "-C", spec["value"], "diff", "--shortstat"] 
            out = subprocess.run(args, capture_output=True, text=True) 
            insertions += getInsertions(out.stdout)
            deletions += getDeletions(out.stdout)

        if(chosen == 2): 
            currPath = f"{random.choice(repoSpecs)["value"]}" 
            currPath += "/"+random.choice([file for file in os.listdir(currPath) if 
                                          os.path.isfile(os.path.join(currPath, file))])
        
    # remove or add lines for to add variation between runs
    if(mode == 2):   
        with open(currPath, "r+") as f: 
            # Return if file is an executable with invalid utf chars
            try: 
                lines = f.readlines()
            except UnicodeDecodeError: 
                return(insertions, deletions)

            if(not currPath in addedPaths):
                repo_name = os.path.basename(os.path.dirname(currPath))
                dest_path = f"{backupDir}/{repo_name}/{os.path.basename(currPath)}"

                # ensure repo folder exists in backup folder
                os.makedirs(os.path.dirname(dest_path), exist_ok = True)
                shutil.copy2(currPath, dest_path)  
                addedPaths[currPath] = dest_path 

            lines = lines[:-1]
            truncLines = "".join(lines)
        with open(currPath, "w") as f: 
            f.write(truncLines)
    else: 
        if(not currPath in addedPaths):
            with open(currPath, "r") as f: 

                # Return if file is an executable with invalid utf chars
                try: 
                    repo_name = os.path.basename(os.path.dirname(currPath))
                    dest_path = f"{backupDir}/{repo_name}/{os.path.basename(currPath)}"

                    # ensure repo folder exists in backup folder
                    os.makedirs(os.path.dirname(dest_path), exist_ok = True)
                    shutil.copy2(currPath, dest_path)  
                    addedPaths[currPath] = dest_path 
                except UnicodeDecodeError: 
                    return(insertions, deletions)

        with open(currPath, "a") as f: 
            f.write(ADD_HEADER)
     
    # Reset files added with intent to add 
    if(("untracked_files" in settings) and (settings["untracked_files"])): 
        if("repositories_directory" in settings): 
            dirs = next(os.walk(settings["repositories_directory"]))[1]
            for dir in dirs:
                # remove all untracked files added to index with 
                # "intent to add" 
                args = [shutil.which("git"), "-C", 
                        f"{settings["repositories_directory"]}/{dir}", 
                        "reset", "--mixed"]
                out = subprocess.run(args, stdout=subprocess.DEVNULL) 

        if("single_repos" in settings): 
            repoSpecs = settings["single_repos"]
            for spec in repoSpecs:
                args = [shutil.which("git"), "-C", spec["value"], "reset", 
                        "--mixed"] 
                out = subprocess.run(args, stdout=subprocess.DEVNULL) 
        
    return(insertions,deletions)

def reset_test_files(added_paths, scenes_dir, scenes_backup_dir, 
                     source_file_backup_dir): 
    """Moves original scenes and source files back to where they came from
        Keyword Arguments:
        
        added_paths: dict for source file mappings 
        scenes_dir: path to scenes directory
        scenes_backup_dir: path to backup scenes location 
        source_file_backup_dir: path to backup source files
    """

    for key in added_paths: 
        shutil.move(added_paths[key], key)  

    shutil.rmtree(source_file_backup_dir)

    shutil.rmtree(scenes_dir, ignore_errors=True)
    shutil.move(scenes_backup_dir, scenes_dir)
    
def run_test(args, sceneIndex) -> testRecap:
    """Run a test on scene
    
    Keyword Arguments: 
        args: argument dict that comes from configargparse
        sceneIndex: index into scene list 
    """
    
    scenesBackupDst = "scenesBackup/scenes" 
    scenesBackupDir = "scenesBackup"
    sourceFileBackupDst = "sourceFileBackup/srcFiles" 
    sourceFileBackupDir = "sourceFileBackup" 
    os.makedirs(sourceFileBackupDst ,exist_ok=True)
    shutil.move(args.filename, scenesBackupDir) 

    # ensure scenes dir exists in case was reset of new obs installation
    os.makedirs(args.filename, exist_ok=True)

    # copy test scene to obs install 
    shutil.copy2(args.scenes[sceneIndex], 
                 args.filename+f"/{os.path.basename(args.scenes[sceneIndex])}")

    try: 
        obs_proc = subprocess.Popen([shutil.which("obs"), 
                                     "--disable-shutdown-check"], 
                                    stdout=subprocess.PIPE, 
                                    stderr=subprocess.PIPE)

        # Give obs a bit of time to startup 
        time.sleep(args.bootTime)

        path = newest(args.logs)

        addedPaths = dict()
        startTime = time.time()
        successes = 0
        failures = 0
        lastTime = -1
        while((time.time() - startTime) < args.runtime):
            if(os.stat(path).st_mtime != lastTime):
                with open(path, "r") as f: 
                    lastLine = tail(f, 1, 1024)[0]
                if(LOG_HEADER in lastLine): 
                    numOnly = lastLine.split(LOG_HEADER, 1)[1]
                    parsed = re.findall('[0-9]+', numOnly)
                    diffOut = getDiffOutput(args.scenes[sceneIndex], 
                                            addedPaths, sourceFileBackupDst)

                    if(diffOut[0] != int(parsed[0]) or 
                       diffOut[1] != int(parsed[1])): 
                        if(args.verbose):
                            print(FAIL + f"[Plugin] INSERTIONS: {parsed[0]} " 
                                  f"DELETIONS: {parsed[1]} != "
                                  f"[Script] INSERTIONS: {diffOut[0]} "
                                  f"DELETIONS: {diffOut[1]}" + RESET) 
                        failures += 1
                    else:
                        if(args.verbose): 
                            print(SUCCESS + f"[SCRIPT] INSERTIONS: {diffOut[0]} " 
                                  f"DELETIONS: {diffOut[1]}" + RESET)
                            print(SUCCESS + f"[PLUGIN] INSERTIONS: {parsed[0]} " 
                                  f"DELETIONS: {parsed[1]}" + RESET)
                        successes += 1

                lastTime = os.stat(path).st_mtime 
            time.sleep(.1)

        # kill obs and wait for process to actually close to grab output 
        obs_proc.send_signal(signal.SIGINT)
        obs_proc.wait()

        output, err= obs_proc.communicate()
        lines = str(output).split("\\n")
        tickLine = [string for string in lines if "git-stats_tick" in string][0]
        min = re.search("min=[0-9]+.[0-9]+|min=[0-9]+", tickLine).group()
        minNum = re.search(r"\d+.\d+|\d+", min).group() 
        max = re.search("max=[0-9]+.[0-9]+|max=[0-9]+", tickLine).group()
        maxNum = re.search(r"\d+.\d+|\d+", max).group() 
        ninetyNineP = re.search("99th percentile=[0-9]+.[0-9]+|99th percentile=[0-9]+", tickLine).group()

        # get match AFTER 99
        ninetyNum = re.findall(r"\d+.\d+|\d+", ninetyNineP)[1]
        median = re.search("median=[0-9]+.[0-9]+|median=[0-9]+", tickLine).group()
        medianNum = re.search(r"\d+.\d+|\d+", median).group()
        
        if(args.outfile): 
            if(not os.path.exists(args.outfile)): 
                fptr = open(args.outfile, "w")
            else: 
                fptr = open(args.outfile, "a")
        else: 
            fptr = None

        if(args.verbose):
            print(f"[AVG PERFORMANCE] " 
                  f"({os.path.basename(args.scenes[sceneIndex])}) " 
                  f"min: {minNum}ms max: {maxNum}ms "
                  f"99th%: {ninetyNum}ms median: {medianNum}ms", file = fptr)  

            print(f"[SUMMARY] " 
                  f"({os.path.basename(args.scenes[sceneIndex])}) "
                  f"{SUCCESS} passed: {successes} "
                  f"{FAIL} failed: {failures}" + RESET, file = fptr)  
        
        if(args.outfile): 
            fptr.close()

    except KeyboardInterrupt as e: 
        reset_test_files(addedPaths, args.filename, scenesBackupDir, 
                         sourceFileBackupDir)
        exit()

    except Exception as e: 
        reset_test_files(addedPaths, args.filename, scenesBackupDir, 
                         sourceFileBackupDir)
        exit()


    reset_test_files(addedPaths, args.filename, scenesBackupDir, 
                     sourceFileBackupDir)

    return(testRecap(minNum, maxNum, ninetyNum, 
                     medianNum, successes, failures))

def main(args):
    
    testRecaps = list()

    currSceneIndex = 0
    sceneLen = len(args.scenes)
    
    if(args.outfile and os.path.exists(args.outfile)): 
        os.remove(args.outfile)

    print(f"Now Testing With Scene " 
          f"{os.path.basename(args.scenes[currSceneIndex])}") 

    recapList = [[] for i in range(sceneLen)] 
     
    for num in range(0, args.numTests*sceneLen):
        if(((num % args.numTests) == 0) and num): 
            currSceneIndex = currSceneIndex + 1 
            print("Now Testing With Scene " 
                  f"{os.path.basename(args.scenes[currSceneIndex])}") 
        recapList[currSceneIndex].append(run_test(args, currSceneIndex))
        time.sleep(args.bootTime)
    
    if(args.outfile):
        fptr = open(args.outfile, "a");
    else: 
        fptr = None
    
    print("\n\n[OVERALL SUMMARIES]\n", file = fptr)

    successes = 0 
    failures = 0
    min = 0
    max = 0
    nine = 0
    median = 0
    for index, recapSet in enumerate(recapList):
        for recap in recapSet:

            for field in fields(recap):
                match field.name: 
                    case "successes": 
                        successes += int(getattr(recap, field.name)) 
                    case "failures": 
                        failures +=  int(getattr(recap, field.name))
                    case "min": 
                        min += float(getattr(recap, field.name))
                    case "max": 
                        max += float(getattr(recap, field.name))
                    case "nineth": 
                        nine += float(getattr(recap, field.name))
                    case "median": 
                        median += float(getattr(recap, field.name))


        print(f"[[{os.path.basename(args.scenes[index]).upper()}]]", 
              file = fptr) 
        print(f"Total Successes: {successes}", file = fptr)
        print(f"Total Failures: {failures}", file = fptr)
        print(f"Min Avg Time: " 
              f"{round(min/len(recapSet), 3)} ms", file = fptr) 
        print(f"Max Avg Time: " 
              f"{round(max/len(recapSet), 3)} ms", file = fptr) 
        print(f"Median Avg Time: " 
            f"{round(median/len(recapSet), 3)} ms", file = fptr) 
        print(f"99th Percentile Avg Time: " 
              f"{round(nine/len(recapSet), 3)} ms\n", file = fptr) 
        
        
    if(args.outfile): 
        fptr.close()

# FIXME: Untracked tests are broken
if __name__ == "__main__":
    parser = configargparse.ArgParser(
        description="Run automated test on git-stats",
        config_file_parser_class=configargparse.YAMLConfigFileParser
    )

    parser.add_argument("--verbose", "-v", default=0, 
                        type=int,
                        help="Enable Verbose Output")

    parser.add_argument("--config", "-c", default="testConfig.yaml", 
                        type=str,
                        is_config_file=True, help="Configuration File Path") 

    parser.add_argument("--filename", 
                        default="~/.config/obs-studio/basic/scenes", 
                        type=str,
                        help="Path To Folder Of Scenes To Temporarily Replace") 

    parser.add_argument("--scenes", "-s", 
                        type=str,
                        required=True,
                        nargs="+",
                        help="Scene description to test with") 

    parser.add_argument("--logs", "-l", 
                        default=f"{os.path.expanduser("~")}/.config/obs-studio/logs/", 
                        type=str,
                        help="Amount of time to sleep to give obs time to boot up") 

    parser.add_argument("--numTests", "-n", type=int, default=10, 
                        help="Number of times to run the test") 

    parser.add_argument("--runtime", "-r", default=60, 
                        type=int,
                        help="Amount of time a single test can run for")

    parser.add_argument("--bootTime", "-b", default=5, 
                        type=int,
                        help="Amount of time to sleep to give obs time to boot up") 
    
    parser.add_argument("--outfile", "-o", default=None, 
                        type=str,
                        help="File to output summary to if desired")
    
    args = parser.parse_args() 
    
    print(SUCCESS + "Testing Git-Stats" + RESET);
    args.filename = os.path.expanduser(args.filename)

    # Allow us to treat SIGINT as an exception
    signal.signal(signal.SIGINT, signal.default_int_handler)
    main(args);
