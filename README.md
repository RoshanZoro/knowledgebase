---

# KB Search 🧠

**A high-performance, multi-threaded Knowledge Base explorer built with C++ and wxWidgets.**

KB Search is a lightning-fast desktop utility designed for sysadmins and engineers to instantly query thousands of technical entries. It features a custom-built UI, asynchronous search processing to keep the interface fluid, and an SQLite FTS5 backend.

## ✨ Features

* **Asynchronous Search Engine**: Search results are processed in a background worker thread using `std::thread`, ensuring the UI never "ghosts" or freezes during heavy queries.
* **Full-Text Search (FTS5)**: Powered by SQLite's FTS5 module for near-instant retrieval across large datasets.
* **Smart Highlighting**: Automatically identifies and highlights search terms within the results.
* **Modern UI Components**:
* **Topic Sidebar**: Quick-filter by category or technology.
* **Intelligent EntryCards**: Beautifully formatted cards that distinguish between CLI commands, step-by-step guides, and prose.
* **One-Click Copy**: Built-in "Copy" and "Copy All" buttons for rapid command execution.


* **Debounced Input**: Optimized search triggering to prevent unnecessary database load while typing.

## 🛠️ Tech Stack

* **Language**: C++17
* **Framework**: [wxWidgets 3.2+](https://www.wxwidgets.org/)
* **Database**: SQLite 3 (with FTS5 enabled)
* **Build System**: CMake

## 🚀 Getting Started

### Prerequisites

* **Windows**: Visual Studio 2022 (MSVC)
* **CMake**: Version 3.16 or higher
* **wxWidgets**: Set up as a system environment variable or included in your project path.

### Installation & Build

1. Clone the repository:
```bash
git clone https://github.com/yourusername/kb_search.git
cd kb_search

```


2. Generate the build files:
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64

```


3. Build the Release version:
```bash
cmake --build build --config Release

```


4. **Database Setup**: Ensure `knowledge_base.db` is placed in the same directory as the generated `.exe`.

## 📂 Project Structure

* `/src`: Core logic, Threading implementation, and UI Event handling.
* `/include`: Header files and class definitions.
* `/assets`: Themes and UI styling constants.
* `sqlite3.c / .h`: Local SQLite implementation with FTS5 flags enabled.

## 📝 License

Distributed under the MIT License. See `LICENSE` for more information.

---
