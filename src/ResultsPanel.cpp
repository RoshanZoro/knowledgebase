#include "ResultsPanel.h"
#include "EntryCard.h"
#include "Theme.h"
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <sstream>
#include <algorithm>
#include <unordered_set>

// ── Construction ──────────────────────────────────────────────────────────────
ResultsPanel::ResultsPanel(wxWindow* parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxVSCROLL | wxBORDER_NONE)
{
    SetBackgroundColour(Theme::BG_APP);
    SetScrollRate(0, 20);

    m_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_sizer);

    ShowWelcome();
}

// ── Clear all cards ───────────────────────────────────────────────────────────
void ResultsPanel::ClearCards() {
    Freeze();
    for (wxWindowList::iterator it = GetChildren().begin();
         it != GetChildren().end(); ) {
        wxWindow* child = *it++;
        m_sizer->Detach(child);
        child->Destroy();
         }
    m_sizer->Clear(false);
    m_headerBar = nullptr;
    Thaw();
}

// ── Build status header bar ───────────────────────────────────────────────────
void ResultsPanel::BuildHeader(const std::string& msg, const wxColour& colour) {
    auto* bar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 44));
    bar->SetBackgroundColour(Theme::BG_PANEL);
    auto* s = new wxBoxSizer(wxHORIZONTAL);

    auto* lbl = new wxStaticText(bar, wxID_ANY, wxString::FromUTF8(msg.c_str()));
    lbl->SetFont(Theme::FontUI(10));
    lbl->SetForegroundColour(colour.IsOk() ? colour : Theme::TXT_SECONDARY);
    lbl->SetBackgroundColour(Theme::BG_PANEL);

    s->AddSpacer(16);
    s->Add(lbl, 0, wxALIGN_CENTER_VERTICAL);
    bar->SetSizer(s);

    m_sizer->Add(bar, 0, wxEXPAND | wxBOTTOM, 2);

    // Bottom border
    auto* border = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    border->SetBackgroundColour(Theme::DIVIDER);
    m_sizer->Add(border, 0, wxEXPAND | wxBOTTOM, Theme::CARD_GAP);

    m_headerBar = bar;
}

// ── Section separator ─────────────────────────────────────────────────────────
wxPanel* ResultsPanel::MakeSeparator(const std::string& label) {
    auto* sep = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 30));
    sep->SetBackgroundColour(Theme::BG_APP);
    auto* s = new wxBoxSizer(wxHORIZONTAL);

    auto* lbl = new wxStaticText(sep, wxID_ANY,
                                  wxString::FromUTF8(label.c_str()).Upper());
    lbl->SetFont(Theme::FontUI(8, true));
    lbl->SetForegroundColour(wxColour(140, 150, 170));
    lbl->SetBackgroundColour(Theme::BG_APP);

    auto* line = new wxPanel(sep, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    line->SetBackgroundColour(Theme::DIVIDER);

    s->AddSpacer(16);
    s->Add(lbl,  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    s->Add(line, 1, wxALIGN_CENTER_VERTICAL);
    s->AddSpacer(16);
    sep->SetSizer(s);
    return sep;
}

// ── Tokenise query for highlighting ──────────────────────────────────────────
static std::vector<std::string> TokeniseQuery(const std::string& q) {
    std::vector<std::string> tokens;
    std::istringstream ss(q);
    std::string word;
    static const std::unordered_set<std::string> skip = {
        "how","to","the","a","an","in","on","for","and","or","is","are","with"
    };
    while (ss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (word.size() >= 2 && skip.find(word) == skip.end())
            tokens.push_back(word);
    }
    return tokens;
}

// ── Show welcome / placeholder ────────────────────────────────────────────────
void ResultsPanel::ShowWelcome() {
    ClearCards();
    Freeze();

    m_sizer->AddSpacer(60);

    auto* ctr = new wxPanel(this, wxID_ANY);
    ctr->SetBackgroundColour(Theme::BG_APP);
    auto* cs = new wxBoxSizer(wxVERTICAL);

    auto* title = new wxStaticText(ctr, wxID_ANY, wxT("Search your knowledge base"));
    title->SetFont(Theme::FontTitle(16));
    title->SetForegroundColour(Theme::TXT_PRIMARY);
    title->SetBackgroundColour(Theme::BG_APP);

    auto* sub = new wxStaticText(ctr, wxID_ANY,
        wxT("Type above to search, or browse topics in the sidebar"));
    sub->SetFont(Theme::FontUI(11));
    sub->SetForegroundColour(Theme::TXT_SECONDARY);
    sub->SetBackgroundColour(Theme::BG_APP);

    auto* hint = new wxStaticText(ctr, wxID_ANY,
        wxT("linux user  •  vlan trunk  •  ospf passive  •  lvm extend  •  join domain"));
    hint->SetFont(Theme::FontMono(9));
    hint->SetForegroundColour(Theme::TXT_ACCENT);
    hint->SetBackgroundColour(Theme::BG_APP);

    cs->Add(title, 0, wxALIGN_CENTER | wxBOTTOM, 10);
    cs->Add(sub,   0, wxALIGN_CENTER | wxBOTTOM, 16);
    cs->Add(hint,  0, wxALIGN_CENTER);
    ctr->SetSizer(cs);

    m_sizer->Add(ctr, 0, wxALIGN_CENTER | wxALL, 20);

    Thaw();
    Layout();
    FitInside();
    Scroll(0, 0);
}

// ── Show no-results ────────────────────────────────────────────────────────────
void ResultsPanel::ShowNoResults(const std::string& query) {
    ClearCards();
    Freeze();

    m_sizer->AddSpacer(50);
    auto* ctr = new wxPanel(this, wxID_ANY);
    ctr->SetBackgroundColour(Theme::BG_APP);
    auto* cs = new wxBoxSizer(wxVERTICAL);

    auto* msg = new wxStaticText(ctr, wxID_ANY,
        wxString::Format(wxT("No results for \"%s\""),
                         wxString::FromUTF8(query.c_str())));
    msg->SetFont(Theme::FontTitle(13));
    msg->SetForegroundColour(Theme::TXT_SECONDARY);
    msg->SetBackgroundColour(Theme::BG_APP);

    auto* tip = new wxStaticText(ctr, wxID_ANY,
        wxT("Tips: try fewer keywords  •  check spelling  •  browse topics sidebar"));
    tip->SetFont(Theme::FontUI(10));
    tip->SetForegroundColour(wxColour(160,170,185));
    tip->SetBackgroundColour(Theme::BG_APP);

    cs->Add(msg, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    cs->Add(tip, 0, wxALIGN_CENTER);
    ctr->SetSizer(cs);

    m_sizer->Add(ctr, 0, wxALIGN_CENTER | wxALL, 20);

    Thaw();
    Layout();
    FitInside();
    Scroll(0, 0);
}

// ── Show search results ────────────────────────────────────────────────────────
void ResultsPanel::ShowResults(const std::vector<Entry>& entries,
                                 const std::string& query,
                                 const std::string& correction) {
    ClearCards();
    m_lastTerms = TokeniseQuery(query);

    Freeze();

    // Status header
    std::string hdrMsg = std::to_string(entries.size()) + " result" +
                         (entries.size() != 1 ? "s" : "") +
                         " for \"" + query + "\"";
    if (!correction.empty()) hdrMsg += "  —  Did you mean: " + correction + "?";
    BuildHeader(hdrMsg, Theme::TXT_SECONDARY);

    // Group by source for visual separation
    std::string curSource;
    int idx = 0;
    for (auto& entry : entries) {
        if (entry.source != curSource && entries.size() > 5) {
            // Add source separator when multiple sources
            long srcCount = std::count_if(entries.begin(), entries.end(),
                [&](const Entry& e){ return e.source == entry.source; });
            if (srcCount > 1)
                m_sizer->Add(MakeSeparator(entry.source), 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
            curSource = entry.source;
        }

        auto* card = new EntryCard(this, entry, m_lastTerms,
                                   idx + 1, (int)entries.size());
        m_sizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,
                     Theme::CARD_GAP);
        ++idx;
    }

    m_sizer->AddSpacer(20);

    Thaw();
    Layout();
    FitInside();
    Scroll(0, 0);
}

// ── Browse mode ────────────────────────────────────────────────────────────────
void ResultsPanel::ShowBrowse(const std::vector<Entry>& entries,
                               const std::string& topicName) {
    ClearCards();
    m_lastTerms.clear();

    Freeze();

    std::string hdrMsg = topicName + "  —  " +
                         std::to_string(entries.size()) + " entr" +
                         (entries.size() != 1 ? "ies" : "y");
    BuildHeader(hdrMsg, Theme::TXT_ACCENT);

    std::string curSection;
    for (auto& entry : entries) {
        if (entry.section != curSection) {
            m_sizer->Add(MakeSeparator(entry.section), 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
            curSection = entry.section;
        }
        auto* card = new EntryCard(this, entry, {}, 0, 0);
        m_sizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, Theme::CARD_GAP);
    }

    m_sizer->AddSpacer(20);

    Thaw();
    Layout();
    FitInside();
    Scroll(0, 0);
}
