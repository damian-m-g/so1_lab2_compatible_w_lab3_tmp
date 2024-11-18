# ShellProject

## How to compile it?

- Clone the repository and `cd` into it.
- Make sure you have Python 3 installed; if not:  
  - `sudo apt update`
  - `sudo apt install python3`
  - `sudo apt install python3-pip`
- Install **conan**:
  - `pip install conan`
- Configure **conan**::
  - `conan profile detect --force`
  - `conan config home`
- Build phase:
  - `mkdir build`
  - `conan install . --build=missing`
  - `cd build`
  - `cmake .. -DCMAKE_TOOLCHAIN_FILE=./Debug/generators/conan_toolchain.cmake`
  - `make`

These steps should have generated the executable file inside the `build` folder. You can execute it from your shell as any other program, without any argument, or with a path to a Batch file (a certain implementation of it, more on this later).

## How to run the tests and coverage report?

- Make sure to perform the steps to compile, mentioned on the section "How to compile it?".
- With current working directory in `./build` (relative to the project root dir), execute:
- `cmake .. -DCMAKE_TOOLCHAIN_FILE=./Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DRUN_COVERAGE=1`
- `make`
- `ctest`
- You can see the results on-screen. A dir named `coverage_report` will be generated on the root dir of the project. Inside, seek for the `index.html` file, and open it with your prefered web explorer to see the lcov run result.

## How to generate the project documentation?

- Make sure to perform the steps to compile, mentioned on the section "How to compile it?".
- Install Doxygen if you don't have it installed on your system; i.e.: in Debian related systems: `sudo apt install doxygen`
- Make sure the current working directory of your shell is the project root dir, and execute:
- `doxygen Doxyfile`
- A dir named `documentation` will be generated on the root dir of the project. Inside, seek for the `index.html` file, and open it with your prefered web explorer to see the API documentation for the project.

## How to use it without initial argument (classic run)?

### Internal Commands

These next commands are home-made for ShellProject:

- `cd`: Change Directory. Every thing you pass after `cd ` (notice the space) is treated as the directory to which you want to change. Do not pass double quotes for paths with spaces in-between, it's not needed. Casting `cd` by its own show the current working directory. Casting `cd -` switches the current directory to the last saved (if) "current" working directory.
- `echo`: Every thing you pass after `echo ` (notice the space) is echoed to the terminal. Accepts global variables as argument, i.e.: `echo $HOME`.
- `clr`: Cleans the terminal. Doesn't receive args.
- `quit`: Exits the program cleanly. Suggested way to end the program. Doesn't receive args.    

#### "metrics" app related internal commands  

ShellProject can interact with the "metrics" app, which you need to have placed in the `/opt/` dir:

- `start_monitor`: Starts the "metrics" app, which uses a JSON file that parses to understand how to execute. _It is recommended to use ` &` (notice the space prefixed) at the end of the command to execute this command in the background (more on this later), otherwise the ShellProject will keep holding until you end the "metrics" app with for example a [Ctrl]+[C] signal_. You can pass several arguments for this (filled with example values, for didactic purposes):
  - `--config=/path/to/existing/json/config/file.json`: This option ignores all of the rest, as you are passing a fixed path to an existing (hopefully) configuration file; hence there's no need to generate a new one.
  - `--update_interval=3`: The update interval of the metrics, in seconds (as an integer only). Default value: 1.
  - `--cpu=false`: Wheter to measure CPU data or not. Default value: true.
  - `--mem=true`: Wheter to measure main memory data or not. Default value: true.
  - `--hdd=false`: Wheter to measure secondary memory (HDD, SSD, etc.) data or not. Default value: true.
  - `--net=true`: Wheter to measure Network data or not. Default value: true.
  - `--procs=false`: Wheter to measure Processes data or not. Default value: true.
- `stop_monitor`: Stops the "metrics" app, if you started it with the ShellProject.
- `status_monitor`: Shows main data from the "metrics" app, if you started it with the ShellProject.

### External Commands

Every other command than the 7 shown, are executed as if you do in your regular Shell.

### Background execution

All commands accept ` &` (notice the space prefixed) at their end. This will make the command to be executed in the background, as feedback, the job id and its process id are shown on screen. Note that despite all commands accepts ` &`, some internal commands ignores it, as they are fast enough to be executed in the foreground. One internal command that for example is suggested to be used with ` &` is `start_monitor &`.

### stdin & stdout redirection

At the end of a command, you can provide the next syntax to redirect the stdin and/or stdout.

- **stdin** can be redirected with `< path/to/some/file_a.txt`.
- **stdout** can be redirected with `> path/to/some/file_b.txt`.

For example: `wc -c < path/to/some/file_a.txt > path/to/some/file_b.txt`.

_NOTE: Internal commands doesn't support stdin redirection; but they do support stdout redirection. They won't fail thou, if you provide the stdin redirection syntax, it will just be ignored._

### Pipes

ShellProject support pipes in-between commands, for example: `ls | grep txt | wc -l`. The output of the first command is connected to the input of the second, and so on.

_NOTE: The internal commands `start_monitor`, `stop_monitor` and `quit` have no support while using pipes. Use them as "solo" commands._

## How to use it with an implementation of a Batch file?

ShellProject is able to accept a unique argument, which has to be a path to a Batch file. It's worth noticing that this Batch file has to actually be a simpler version of a Batch file, meaning:

- No comments allowed.
- One line per command (i.e.: don't use ";" to separate commands on one line).

ShellProject will execute each line of it as if you were typing it in a _classic run_.
























