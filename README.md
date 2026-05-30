# Aggie-Shell

A Unix shell implemented from scratch in C++, built as a deep dive into how shells actually work under the hood.

## Features

- **Command execution** — forks child processes and uses `execvp` to run any system command
- **Piping** — chains multiple commands with `|`, wiring stdout of one process to stdin of the next
- **I/O redirection** — supports `<` (input from file) and `>` (output to file)
- **Background processes** — run commands with `&`; the shell continues accepting input while they execute and reaps them automatically when they finish
- **`cd` with history** — navigate directories, including `cd -` to jump back to the previous directory
- **Colored prompt** — displays current timestamp, username, and working directory

## Architecture

| File | Responsibility |
|------|---------------|
| `shell.cpp` | Main REPL loop, process forking, pipe/redirect wiring |
| `Tokenizer.cpp/.h` | Lexes raw input into a list of `Command` objects; handles quoted strings |
| `Command.cpp/.h` | Parses a single command's arguments, input/output filenames, and background flag |

## Build & Run

```bash
g++ -o shell shell.cpp Tokenizer.cpp Command.cpp
./shell
```

## Example

```
12:34:56 yourname /home/yourname$ ls -la | grep cpp
12:34:56 yourname /home/yourname$ cat input.txt | grep "hello" > output.txt
12:34:56 yourname /home/yourname$ sleep 10 &
12:34:56 yourname /home/yourname$ cd -
```
