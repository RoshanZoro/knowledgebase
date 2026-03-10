#pragma once
#include <string>
#include <vector>

// ── Entry ─────────────────────────────────────────────────────────────────────
struct Entry {
    int         id         = 0;
    std::string source;
    std::string section;
    std::string subsection;
    std::string type;       // "command" | "steps" | "prose"
    std::string content;
    std::string tags;
    double      weight     = 0.5;
    double      score      = 0.0;   // set at search time
};

// ── Topic ─────────────────────────────────────────────────────────────────────
struct Topic {
    int         id       = 0;
    std::string category;
    std::string display;
    std::string query;
    std::string source;
    std::string section;
};

// ── Category (grouped topics) ─────────────────────────────────────────────────
struct Category {
    std::string        name;
    std::vector<Topic> topics;
};

// ── Search options ────────────────────────────────────────────────────────────
struct SearchOptions {
    bool commandOnly = false;
    bool stepsOnly   = false;
    int  maxResults  = 40;
};
