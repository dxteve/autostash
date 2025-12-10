# 💾 AUTOSTASH: Multithreaded C Backup Utility

AUTOSTASH is a simple, multithreaded command-line utility written in C that automatically backs up specified directories to a timestamped location on a recurring schedule. It is designed to run efficiently on Unix-like operating systems (Linux and macOS) using POSIX threads (pthread).

## 🌟 Features

* **Multithreaded Backups:** Each configured source folder is backed up concurrently using separate threads for speed.
* **Timestamped History:** Backups are organized into directories named by the date and time (`YYYY-MM-DD_HH-MM-SS`).
* **Simple CLI Menu:** Easy-to-use text-based menu for adding/removing folders and controlling the backup cycle.
* **Portable (POSIX):** Highly compatible with Ubuntu, Debian, macOS, and environments like WSL.
* **Recursive Copying:** Copies files and subdirectories accurately from source to destination.

## 🛠️ Requirements

To build and run this project, you need a system with the following:

* A **Unix-like environment** (Linux, macOS, or WSL on Windows).
* The **GNU Compiler Collection (GCC)** or a compatible C compiler (like Clang).
* The **`make`** utility.
* The **POSIX Threads library (`-lpthread`)** for compilation.

---

## ⚙️ Build and Run

1.  **Clone the repository** and navigate into the directory.
2.  **Compile** the project using the provided `Makefile`:
    ```bash
    make
    ```
3.  **Run** the executable:
    ```bash
    ./autostash
    ```