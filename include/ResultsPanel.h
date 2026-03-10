#pragma once
#include "Types.h"
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <vector>
#include <string>

// ── ResultsPanel ─────────────────────────────────────────────────────────────
// A scrollable panel that holds and lays out EntryCard widgets.
// Owns the cards — clears and rebuilds on each new search result set.
class ResultsPanel : public wxScrolledWindow {
public:
    explicit ResultsPanel(wxWindow* parent);

    // Replace current cards with new results
    void ShowResults(const std::vector<Entry>& entries,
                     const std::string& query,
                     const std::string& correction = "");

    // Show "no results" message
    void ShowNoResults(const std::string& query);

    // Show placeholder / welcome
    void ShowWelcome();

    // Show all entries from a topic (browse mode)
    void ShowBrowse(const std::vector<Entry>& entries,
                    const std::string& topicName);

private:
    wxBoxSizer* m_sizer      = nullptr;
    wxPanel*    m_headerBar  = nullptr;

    void ClearCards();
    void BuildHeader(const std::string& msg,
                     const wxColour& colour = wxNullColour);
    wxPanel* MakeSeparator(const std::string& label);

    std::vector<std::string> m_lastTerms;
};
