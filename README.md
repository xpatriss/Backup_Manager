# Backup Manager

A Linux command-line backup tool written in C that watches a source directory and keeps one or more backup directories synchronized using the inotify API.

The program runs as an interactive shell: you add backup jobs, list active jobs, stop selected jobs, restore data from a backup, and exit cleanly.

## Features

- Real-time monitoring of directory changes using inotify
- Support for multiple backup targets for the same source directory
- Initial full copy of the source tree into each target
- Continuous synchronization for created, deleted, renamed, and modified files
- Backup job tracking and cleanup via a simple in-memory map
- Restore functionality from a backup target to the source path
- Logging of actions to actions.log

## Requirements

- Linux operating system
- GCC or Clang
- GNU Make
- Kernel support for inotify

## Build

From the project root:

```bash
make
```

To remove compiled objects and the binary:

```bash
make clean
```

This will produce the executable:

```bash
./backup_app
```

## Usage

When the program starts, it prints a short command list and waits for input.

### Commands

| Command | Syntax | Description |
| --- | --- | --- |
| add | add <source> <target1> [target2 ...] | Starts monitoring the source directory and creates a backup in each target directory |
| end | end <source> <target1> [target2 ...] | Stops synchronization for the specified target(s) |
| restore | restore <source> <target> | Restores the directory tree from a backup target |
| list | list | Shows all active backup jobs |
| exit | exit | Stops the program and cleans up running backup processes |

### Example

```bash
./backup_app

add /home/user/project /mnt/backup/one /mnt/backup/two
list
restore /home/user/project /mnt/backup/one
end /home/user/project /mnt/backup/one
exit
```

## Notes

- The source directory must exist before running add.
- A target directory cannot be inside the source directory.
- A target path can only be used once for a given source.
- Multiple target paths can be provided in one add command.
- The application writes operational information to actions.log in the project directory.

## Project layout

- src/main.c: interactive command loop and process lifecycle
- src/add_command.c / add_command.h: add job logic
- src/end_command.c / end_command.h: stop job logic
- src/list_command.c / list_command.h: list active jobs
- src/restore.c / restore.h: restore backup content
- src/watcher.c / watcher.h: directory monitoring and sync behavior
- src/parse.c / parse.h: command parsing
- src/common.c / common.h: shared helpers and logging
- src/copy.c / copy.h: file copy logic
- src/delete.c / delete.h: deletion handling

## Behavior summary

The manager creates child processes for each backup job, and each child watches the source directory for filesystem events. When changes are detected, the program keeps the backup tree in sync. The main process handles command input, keeps the list of active mappings, and can clean up jobs when requested.

## Important warning

This project is designed for Linux and relies on POSIX and inotify behavior. It is not meant to be used on Windows or macOS without additional compatibility changes.

## Author

- **Patrycja Borkowska** – [GitHub Profile](https://github.com/xpatriss)

## License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.