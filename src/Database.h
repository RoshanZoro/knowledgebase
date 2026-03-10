#pragma once
#include "Types.h"
#include <string>
#include <vector>

// Forward declaration of the SQLite handle to keep headers clean
struct sqlite3;
struct sqlite3_stmt;

class Database {
public:
    Database() = default;
    ~Database();

    // ── Connection ──────────────────────────────────────────────────────────
    bool Open(const std::string& path);
    void Close();
    bool IsOpen() const { return m_db != nullptr; }

    // Returns the last SQLite error message
    std::string LastError() const { return m_lastError; }

    // ── Queries ─────────────────────────────────────────────────────────────
    // Core FTS5 search - optimized for cheatsheet/command lookup
    std::vector<Entry> SearchFTS(const std::string& query,
        bool commandOnly = false,
        bool stepsOnly = false,
        int  limit = 40) const;

    // Browse mode: fetch everything for a specific source (e.g., "linux")
    std::vector<Entry> GetBySource(const std::string& source) const;

    // Sidebar: fetch categories and their nested topics
    std::vector<Category> GetCategories() const;

    // Metadata: fetch entry counts or build dates
    std::string GetMeta(const std::string& key) const;

private:
    sqlite3* m_db = nullptr;

    // 'mutable' allows us to update the error message even in 'const' methods
    mutable std::string m_lastError;

    // Internal helper for safe UTF-8 string extraction from SQLite
    std::string GetColumnText(sqlite3_stmt* stmt, int col) const;

    // Sets the error message from the SQLite engine
    void SetError(const std::string& err) const { m_lastError = err; }
};