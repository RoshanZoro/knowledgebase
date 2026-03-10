

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

## 🛠️ Prerequisites & Installation

### 1. Install Dependencies

The easiest way to manage **wxWidgets** on Windows is using [vcpkg](https://github.com/microsoft/vcpkg).

```bash
# Clone vcpkg if you don't have it
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install wxWidgets (x64 Windows)
.\vcpkg\vcpkg install wxwidgets:x64-windows

```

### 2. Build the Project

1. Clone this repository:
```bash
git clone https://github.com/RoshanZoro/knowledgebase.git
cd knowledgebase

```


2. Configure with CMake (pointing to your vcpkg toolchain):
```bash
# Replace [path_to_vcpkg] with your actual vcpkg installation path
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64

```


3. Compile the Release binary:
```bash
cmake --build build --config Release

```



## 🚀 Getting Started

Once built, ensure your `knowledge_base.db` is located in the same directory as the executable.

* **Search**: Just start typing. The 250ms debounce and background thread will handle the rest.
* **Filter**: Use the "Commands Only" checkbox to strip away prose and focus on CLI syntax.
* **Copy**: Click "Copy" on any command line to send it to your clipboard instantly.

## 📂 Project Structure

* `/src`: UI Event handling and Threading implementation.
* `/include`: Header files and class definitions.
* `/assets`: Theme constants and UI styling.
* `sqlite3.c / .h`: Local SQLite implementation with `SQLITE_ENABLE_FTS5` enabled for maximum performance.

---

