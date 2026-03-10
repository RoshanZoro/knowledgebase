#include "Database.h"
extern "C" {
#include <sqlite3.h>
}
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>

Database::~Database() { Close(); }

bool Database::Open(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        // m_lastError is updated via Database.h's SetError
        return false;
    }

    // ── PERFORMANCE TUNING ──
    sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA synchronous=OFF;", nullptr, nullptr, nullptr);

    // ── SYNC SEARCH INDEX ──
    // Ensures the virtual FTS5 table is mirroring the latest entries
    sqlite3_exec(m_db, "INSERT INTO entries_fts(entries_fts) VALUES('rebuild');", nullptr, nullptr, nullptr);

    return true;
}

void Database::Close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

std::vector<Entry> Database::SearchFTS(const std::string& query, bool commandOnly, bool stepsOnly, int limit) const {
    std::vector<Entry> out;
    if (!m_db || query.empty()) return out;

    // 1. Clean query for networking terms (treat / as a separator)
    std::string clean = query;
    std::replace(clean.begin(), clean.end(), '/', ' ');
    std::replace(clean.begin(), clean.end(), '"', ' ');

    std::stringstream ss(clean);
    std::string word;
    std::vector<std::string> words;
    while (ss >> word) {
        if (!word.empty()) {
            // Wrap in quotes to escape dashes, add * for prefix matching
            words.push_back("\"" + word + "\"*");
        }
    }

    if (words.empty()) return out;

    // 2. Search Execution Logic
    auto runSearch = [&](const std::string& matchExpr) -> bool {
        std::stringstream sql;
        // JOIN allows us to get the rank/score from FTS but the real data from 'entries'
        sql << "SELECT e.rowid, e.source, e.section, e.subsection, e.type, e.content, e.tags, e.weight "
            << "FROM entries e "
            << "JOIN entries_fts f ON e.rowid = f.rowid "
            << "WHERE entries_fts MATCH ? ";

        if (commandOnly) sql << "AND e.type = 'command' ";
        if (stepsOnly)   sql << "AND e.type = 'steps' ";

        sql << "ORDER BY rank LIMIT " << limit << ";";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, matchExpr.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Entry e;
            e.id = sqlite3_column_int(stmt, 0);
            e.source = GetColumnText(stmt, 1);
            e.section = GetColumnText(stmt, 2);
            e.subsection = GetColumnText(stmt, 3);
            e.type = GetColumnText(stmt, 4);
            e.content = GetColumnText(stmt, 5);
            e.tags = GetColumnText(stmt, 6);
            e.weight = sqlite3_column_double(stmt, 7);
            out.push_back(std::move(e));
        }
        sqlite3_finalize(stmt);
        return !out.empty();
        };

    // 3. Try "AND" search first (strict)
    std::string andQuery;
    for (size_t i = 0; i < words.size(); ++i) {
        andQuery += words[i] + (i == words.size() - 1 ? "" : " AND ");
    }

    if (!runSearch(andQuery)) {
        // 4. Try "OR" search if AND yields nothing (relaxed)
        std::string orQuery;
        for (size_t i = 0; i < words.size(); ++i) {
            orQuery += words[i] + (i == words.size() - 1 ? "" : " OR ");
        }
        runSearch(orQuery);
    }

    return out;
}

std::vector<Entry> Database::GetBySource(const std::string& source) const {
    std::vector<Entry> out;
    const char* sql = "SELECT rowid, source, section, subsection, type, content, tags, weight "
        "FROM entries WHERE source = ? ORDER BY rowid ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, source.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Entry e;
            e.id = sqlite3_column_int(stmt, 0);
            e.source = GetColumnText(stmt, 1);
            e.section = GetColumnText(stmt, 2);
            e.subsection = GetColumnText(stmt, 3);
            e.type = GetColumnText(stmt, 4);
            e.content = GetColumnText(stmt, 5);
            e.tags = GetColumnText(stmt, 6);
            e.weight = sqlite3_column_double(stmt, 7);
            out.push_back(std::move(e));
        }
    }
    sqlite3_finalize(stmt);
    return out;
}

std::vector<Category> Database::GetCategories() const {
    std::vector<Category> out;
    const char* sql = "SELECT id, category, display, query, source, section "
        "FROM topics ORDER BY category ASC, id ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string lastCat = "";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string catName = GetColumnText(stmt, 1);
            if (catName != lastCat) {
                out.push_back({ catName, {} });
                lastCat = catName;
            }
            Topic t;
            t.id = sqlite3_column_int(stmt, 0);
            t.category = catName;
            t.display = GetColumnText(stmt, 2);
            t.query = GetColumnText(stmt, 3);
            t.source = GetColumnText(stmt, 4);
            t.section = GetColumnText(stmt, 5);
            out.back().topics.push_back(std::move(t));
        }
    }
    sqlite3_finalize(stmt);
    return out;
}

std::string Database::GetMeta(const std::string& key) const {
    const char* sql = "SELECT value FROM meta WHERE key = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    std::string result = "";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = GetColumnText(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

std::string Database::GetColumnText(sqlite3_stmt* stmt, int col) const {
    const unsigned char* txt = sqlite3_column_text(stmt, col);
    return txt ? std::string(reinterpret_cast<const char*>(txt)) : "";
}