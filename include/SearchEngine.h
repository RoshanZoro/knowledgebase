#pragma once
#include "Types.h"
#include "Database.h"
#include <string>
#include <vector>

// ── SearchEngine ─────────────────────────────────────────────────────────────
// Sits between the UI and the Database.
// Handles query rewriting, synonym expansion, FTS fallback, and scoring.
class SearchEngine {
public:
    explicit SearchEngine(Database& db) : m_db(db) {}

    // Main search — returns scored, sorted entries
    std::vector<Entry> Search(const std::string& query,
                               const SearchOptions& opts = {}) const;

    // Topic browse — all entries from a source, in document order
    std::vector<Entry> Browse(const std::string& source) const;

    // Last "did you mean" correction from most recent Search()
    std::string LastCorrection() const { return m_lastCorrection; }

private:
    Database&           m_db;
    mutable std::string m_lastCorrection;

    std::string  RewriteQuery(const std::string& q) const;
    std::string  ExpandQuery(const std::string& q) const;
    double       ScoreEntry(const Entry& e,
                            const std::vector<std::string>& terms) const;
    std::vector<std::string> Tokenise(const std::string& q) const;
};
