#pragma once
#include "Database.h"
#include "SearchEngine.h"
#include "ResultsPanel.h"
#include "TopicPanel.h"
#include "Theme.h"
#include <wx/wx.h>
#include <wx/srchctrl.h>
#include <wx/splitter.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <vector>
#include <string>

// ── Event IDs ─────────────────────────────────────────────────────────────────
enum {
    ID_SEARCH_CTRL = wxID_HIGHEST + 1,
    ID_CMD_FILTER,
    ID_SEARCH_TIMER,
    ID_STATUS_TIMER,
    ID_SEARCH_FINISHED // Added for thread communication
};

// ── MainFrame ─────────────────────────────────────────────────────────────────
class MainFrame : public wxFrame {
public:
    MainFrame();

private:
    // ── Core objects ──────────────────────────────────────────────────────────
    Database      m_db;
    SearchEngine* m_engine = nullptr;

    // ── UI panels ─────────────────────────────────────────────────────────────
    wxPanel* m_topBar = nullptr;
    wxSearchCtrl* m_searchCtrl = nullptr;
    wxCheckBox* m_cmdFilter = nullptr;
    TopicPanel* m_topicPanel = nullptr;
    ResultsPanel* m_results = nullptr;
    wxStaticText* m_statusTxt = nullptr;
    wxPanel* m_rightPanel = nullptr;

    // Debounce timer (fires 250ms after last keystroke)
    wxTimer        m_searchTimer;
    wxTimer        m_statusTimer;

    // ── Background Thread Storage ─────────────────────────────────────────────
    // These hold the results while moving from the worker thread to the UI
    std::vector<Entry> m_latestEntries;
    std::string        m_latestQuery;
    std::string        m_latestCorrection;

    // ── Init ──────────────────────────────────────────────────────────────────
    void BuildUI();
    void BuildTopBar(wxPanel* parent, wxBoxSizer* sizer);
    bool LoadDatabase();

    // ── Event handlers ────────────────────────────────────────────────────────
    void OnSearchText(wxCommandEvent& e);
    void OnSearchEnter(wxCommandEvent& e);
    void OnSearchTimer(wxTimerEvent& e);
    void OnCmdFilter(wxCommandEvent& e);
    void OnTopicSelect(const Topic& topic);
    void OnStatusTimer(wxTimerEvent& e);

    // Handler for when the background search thread finishes
    void OnSearchFinished(wxThreadEvent& event);

    // ── Search ────────────────────────────────────────────────────────────────
    void RunSearch(const std::string& query);
    void SetStatus(const std::string& msg, const wxColour& col = wxNullColour);

    wxDECLARE_EVENT_TABLE();
};