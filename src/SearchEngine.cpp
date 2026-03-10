#include "SearchEngine.h"
#include <algorithm>
#include <sstream>
#include <regex>
#include <cctype>
#include <unordered_map>  // <--- Add this
#include <unordered_set>  // <--- Add this

static const std::unordered_set<std::string> FILLER = {
    "how","to","the","a","an","in","on","at","of","for","and","or",
    "is","are","into","onto","by","with","do","can","i","my","its",
    "please","what","where","show","me","give","find","get","list",
    "step","way","ways","using","use","about","tell"
};

static const std::unordered_map<std::string, std::string> SYNONYMS = {
    {"add", "create new install useradd"},
    {"create", "add new make setup install"},
    {"remove", "delete uninstall userdel"},
    {"delete", "remove userdel erase"},
    {"install", "setup deploy enable dnf"},
    {"lvm", "logical volume pv vg lv extend"},
    {"vlan", "trunk access switchport lan"},
    {"linux", "unix bash shell dnf rpm rocky"}
    // ... add any others you had before ...
};
// ── Improved Scoring Constants ──────────────────────────────────────────────
const double SCORE_TITLE_MATCH = 20.0;
const double SCORE_TAG_MATCH = 10.0;
const double SCORE_CONTENT_MATCH = 3.0;
const double SCORE_COMMAND_BOOST = 5.0; // Commands are usually what people want

// ── Tokenizer: Cleans and prepares search terms ─────────────────────────────
std::vector<std::string> SearchEngine::Tokenise(const std::string& q) const {
    std::vector<std::string> tokens;
    std::istringstream ss(q);
    std::string word;
    while (ss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        // Remove non-alphanumeric except for common command chars like / or -
        word.erase(std::remove_if(word.begin(), word.end(), [](char c) {
            return !std::isalnum((unsigned char)c) && c != '/' && c != '-';
            }), word.end());

        if (word.size() >= 2 && FILLER.find(word) == FILLER.end()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

// ── Expand Query: Combines synonyms into a single search string ─────────────
std::string SearchEngine::ExpandQuery(const std::string& q) const {
    auto tokens = Tokenise(q);
    std::string expanded = q;
    for (const auto& t : tokens) {
        auto it = SYNONYMS.find(t);
        if (it != SYNONYMS.end()) {
            expanded += " " + it->second;
        }
    }
    return expanded;
}

// ── Score Entry: The "Brain" of the search ──────────────────────────────────
double SearchEngine::ScoreEntry(const Entry& e, const std::vector<std::string>& terms) const {
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
        };

    std::string title = lower(e.section + " " + e.subsection);
    std::string content = lower(e.content);
    std::string tags = lower(e.tags);

    double score = 0.0;
    for (const auto& t : terms) {
        // High priority: Title/Headline
        if (title.find(t) != std::string::npos) score += SCORE_TITLE_MATCH;

        // Medium priority: Tags
        if (tags.find(t) != std::string::npos) score += SCORE_TAG_MATCH;

        // Lower priority: Body content
        if (content.find(t) != std::string::npos) score += SCORE_CONTENT_MATCH;
    }

    // Boost for exact phrase matches in content
    if (terms.size() > 1) {
        std::string originalQuery;
        for (const auto& t : terms) originalQuery += t + " ";
        if (content.find(originalQuery.substr(0, originalQuery.size() - 1)) != std::string::npos) {
            score += 15.0;
        }
    }

    // Boost "Command" types as they are usually the primary goal of cheatsheets
    if (e.type == "command") score += SCORE_COMMAND_BOOST;

    // Multiply by the manual 'weight' assigned in the DB (0.5 to 2.0 range)
    return score * (e.weight > 0 ? e.weight : 1.0);
}

// ── Main Search Function ────────────────────────────────────────────────────
std::vector<Entry> SearchEngine::Search(const std::string& query, const SearchOptions& opts) const {
    if (query.empty()) return {};

    // 1. Rewrite based on Intent (e.g., "how to..." -> "install...")
    std::string cleanQuery = RewriteQuery(query);

    // 2. Expand with Synonyms (e.g., "delete" -> "remove, uninstall...")
    std::string expanded = ExpandQuery(cleanQuery);

    // 3. Query the Database FTS5 Index
    auto results = m_db.SearchFTS(expanded, opts.commandOnly, opts.stepsOnly, opts.maxResults * 2);

    // 4. If expanded search failed, try the literal search
    if (results.empty()) {
        results = m_db.SearchFTS(cleanQuery, opts.commandOnly, opts.stepsOnly, opts.maxResults);
    }

    // 5. Re-rank based on our advanced scoring logic
    auto searchTerms = Tokenise(cleanQuery);
    for (auto& e : results) {
        e.score = ScoreEntry(e, searchTerms);
    }

    // 6. Sort by descending score
    std::sort(results.begin(), results.end(), [](const Entry& a, const Entry& b) {
        return a.score > b.score;
        });

    // 7. Limit to user-requested count
    if ((int)results.size() > opts.maxResults) {
        results.resize(opts.maxResults);
    }

    return results;
}
// ── Browse Function ──────────────────────────────────────────────────────────
// This handles clicking a topic in the sidebar.
std::vector<Entry> SearchEngine::Browse(const std::string& source) const {
    return m_db.GetBySource(source);
}

// ── Rewrite Query Function ────────────────────────────────────────────────────
// This simplifies "how to install" into just "install" using Regex.
std::string SearchEngine::RewriteQuery(const std::string& q) const {
    std::string lower = q;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // You can keep this simple or add back your INTENT_MAP logic here
    // For now, let's just return the lowercased query so the build passes
    return lower;
}