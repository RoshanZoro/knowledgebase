#include "MainFrame.h"
#include "Theme.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <algorithm>
#include <thread> // Added for background processing



wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_TIMER(ID_SEARCH_TIMER, MainFrame::OnSearchTimer)
EVT_TIMER(ID_STATUS_TIMER, MainFrame::OnStatusTimer)
wxEND_EVENT_TABLE()

// ── Construction ──────────────────────────────────────────────────────────────
MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, wxT("KB Search"),
        wxDefaultPosition, wxSize(1200, 800)),
    m_searchTimer(this, ID_SEARCH_TIMER),
    m_statusTimer(this, ID_STATUS_TIMER)
{
    SetBackgroundColour(Theme::BG_APP);
    SetMinSize(wxSize(900, 600));

    BuildUI();
    Centre();

    if (!LoadDatabase()) {
        SetStatus(wxString::FromUTF8("knowledge_base.db not found — place it next to the .exe").ToStdString(), *wxRED);
    }
    else {
        std::string count = m_db.GetMeta("entry_count");
        std::string built = m_db.GetMeta("built_at");
        SetStatus("Ready  |  " + count + " entries  |  built " + built,
            Theme::TXT_SECONDARY);
    }
}

// ── UI layout ─────────────────────────────────────────────────────────────────
void MainFrame::BuildUI() {
    auto* root = new wxBoxSizer(wxVERTICAL);

    m_topBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, Theme::TOPBAR_H));
    m_topBar->SetBackgroundColour(Theme::BG_TOPBAR);
    BuildTopBar(m_topBar, root);

    auto* bodySizer = new wxBoxSizer(wxHORIZONTAL);

    m_topicPanel = new TopicPanel(this);
    m_topicPanel->SetMinSize(wxSize(Theme::SIDEBAR_W, -1));
    m_topicPanel->SetMaxSize(wxSize(Theme::SIDEBAR_W, -1));
    m_topicPanel->SetCallback([this](const Topic& t) { OnTopicSelect(t); });

    m_rightPanel = new wxPanel(this, wxID_ANY);
    m_rightPanel->SetBackgroundColour(Theme::BG_APP);
    auto* rightSizer = new wxBoxSizer(wxVERTICAL);

    m_results = new ResultsPanel(m_rightPanel);
    rightSizer->Add(m_results, 1, wxEXPAND);

    auto* statusBar = new wxPanel(m_rightPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 26));
    statusBar->SetBackgroundColour(wxColour(235, 237, 243));
    auto* sbSizer = new wxBoxSizer(wxHORIZONTAL);

    m_statusTxt = new wxStaticText(statusBar, wxID_ANY, wxT("Loading…"));
    m_statusTxt->SetFont(Theme::FontUI(9));
    m_statusTxt->SetForegroundColour(Theme::TXT_SECONDARY);
    m_statusTxt->SetBackgroundColour(wxColour(235, 237, 243));

    sbSizer->AddSpacer(12);
    sbSizer->Add(m_statusTxt, 0, wxALIGN_CENTER_VERTICAL);
    statusBar->SetSizer(sbSizer);

    rightSizer->Add(statusBar, 0, wxEXPAND);
    m_rightPanel->SetSizer(rightSizer);

    auto* divider = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(1, -1));
    divider->SetBackgroundColour(wxColour(200, 205, 215));

    bodySizer->Add(m_topicPanel, 0, wxEXPAND);
    bodySizer->Add(divider, 0, wxEXPAND);
    bodySizer->Add(m_rightPanel, 1, wxEXPAND);

    root->Add(bodySizer, 1, wxEXPAND);
    SetSizer(root);
    Layout();
}

void MainFrame::BuildTopBar(wxPanel* parent, wxBoxSizer* rootSizer) {
    auto* tbs = new wxBoxSizer(wxHORIZONTAL);

    m_searchCtrl = new wxSearchCtrl(parent, ID_SEARCH_CTRL, wxEmptyString, wxDefaultPosition, wxSize(520, Theme::SEARCH_H));
    m_searchCtrl->SetFont(Theme::FontUI(11));
    m_searchCtrl->SetDescriptiveText(wxString::FromUTF8("Search — linux user, vlan trunk, ospf, lvm extend…"));
    m_searchCtrl->ShowSearchButton(true);
    m_searchCtrl->ShowCancelButton(true);

    m_cmdFilter = new wxCheckBox(parent, ID_CMD_FILTER, wxT("Commands only"));
    m_cmdFilter->SetFont(Theme::FontUI(10));
    m_cmdFilter->SetForegroundColour(*wxWHITE);
    m_cmdFilter->SetBackgroundColour(Theme::BG_TOPBAR);

    tbs->AddSpacer(Theme::SIDEBAR_W + 8);
    tbs->Add(m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 8);
    tbs->AddSpacer(20);
    tbs->Add(m_cmdFilter, 0, wxALIGN_CENTER_VERTICAL);
    tbs->AddStretchSpacer(1);

    auto* verLbl = new wxStaticText(parent, wxID_ANY, wxT("v1.0"));
    verLbl->SetFont(Theme::FontUI(9));
    verLbl->SetForegroundColour(wxColour(100, 110, 130));
    verLbl->SetBackgroundColour(Theme::BG_TOPBAR);
    tbs->Add(verLbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);

    parent->SetSizer(tbs);
    rootSizer->Add(parent, 0, wxEXPAND);

    // Bindings
    m_searchCtrl->Bind(wxEVT_TEXT, &MainFrame::OnSearchText, this);
    m_searchCtrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::OnSearchEnter, this);
    m_searchCtrl->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnSearchEnter, this);
    m_cmdFilter->Bind(wxEVT_CHECKBOX, &MainFrame::OnCmdFilter, this);

    // Bind the background thread completion event
    this->Bind(wxEVT_THREAD, &MainFrame::OnSearchFinished, this, ID_SEARCH_FINISHED);
}

// ── Load database ─────────────────────────────────────────────────────────────
bool MainFrame::LoadDatabase() {
    wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
    wxString dbPath = exePath.GetPath() + wxFILE_SEP_PATH + wxT("knowledge_base.db");

    if (!wxFileExists(dbPath)) dbPath = wxT("knowledge_base.db");
    if (!m_db.Open(std::string(dbPath.mb_str()))) return false;

    m_engine = new SearchEngine(m_db);
    auto cats = m_db.GetCategories();
    m_topicPanel->LoadCategories(cats);
    return true;
}

// ── Events ────────────────────────────────────────────────────────────────────
void MainFrame::OnSearchText(wxCommandEvent& e) {
    m_searchTimer.Stop();
    if (!m_searchCtrl->GetValue().IsEmpty())
        m_searchTimer.StartOnce(250);
    else
        m_results->ShowWelcome();
    e.Skip();
}

void MainFrame::OnSearchEnter(wxCommandEvent&) {
    m_searchTimer.Stop();
    RunSearch(std::string(m_searchCtrl->GetValue().utf8_str()));
}

void MainFrame::OnSearchTimer(wxTimerEvent&) {
    RunSearch(std::string(m_searchCtrl->GetValue().utf8_str()));
}

void MainFrame::OnCmdFilter(wxCommandEvent&) {
    wxString q = m_searchCtrl->GetValue();
    if (!q.IsEmpty()) RunSearch(std::string(q.mb_str()));
}

void MainFrame::OnTopicSelect(const Topic& topic) {
    m_searchCtrl->SetValue(wxEmptyString);
    if (!m_engine) return;

    auto entries = m_engine->Browse(topic.source);
    if (entries.empty()) {
        SearchOptions opts;
        opts.commandOnly = m_cmdFilter->IsChecked();
        opts.maxResults = 200;
        entries = m_engine->Search(topic.query, opts);
    }

    m_results->ShowBrowse(entries, topic.display);
    SetStatus("Browsing: " + topic.display + "  |  " + std::to_string(entries.size()) + " entries", Theme::TXT_ACCENT);
}

// ── The Threaded Search Logic ─────────────────────────────────────────────────
void MainFrame::RunSearch(const std::string& query) {
    if (query.size() < 2) {
        m_results->ShowWelcome();
        return;
    }
    if (!m_engine) return;

    m_topicPanel->ClearSelection();

    // 1. Show Spinner & Searching State
    SetCursor(*wxHOURGLASS_CURSOR);
    SetStatus("Searching...", Theme::TXT_ACCENT);

    SearchOptions opts;
    opts.commandOnly = m_cmdFilter->IsChecked();
    opts.maxResults = 40;

    // 2. Spawn Background Thread
    std::thread([this, query, opts]() {
        auto entries = m_engine->Search(query, opts);
        std::string correction = m_engine->LastCorrection();

        // Pass results back to main thread via member variables
        this->m_latestEntries = entries;
        this->m_latestQuery = query;
        this->m_latestCorrection = correction;

        // Notify the UI thread
        wxThreadEvent* evt = new wxThreadEvent(wxEVT_THREAD, ID_SEARCH_FINISHED);
        wxQueueEvent(this, evt);
        }).detach();
}

void MainFrame::OnSearchFinished(wxThreadEvent& event) {
    // 3. Reset Cursor & Update UI
    SetCursor(wxNullCursor);

    if (m_latestEntries.empty()) {
        m_results->ShowNoResults(m_latestQuery);
    }
    else {
        m_results->ShowResults(m_latestEntries, m_latestQuery, m_latestCorrection);
    }

    SetStatus(std::to_string(m_latestEntries.size()) + " result" +
        (m_latestEntries.size() != 1 ? "s" : "") +
        " for \"" + m_latestQuery + "\"",
        m_latestEntries.empty() ? wxColour(200, 100, 80) : Theme::TXT_SECONDARY);
}

void MainFrame::SetStatus(const std::string& msg, const wxColour& col) {
    if (!m_statusTxt) return;
    m_statusTxt->SetLabel(wxString::FromUTF8(msg.c_str()));
    if (col.IsOk()) m_statusTxt->SetForegroundColour(col);
    m_statusTxt->GetParent()->Layout();
}

void MainFrame::OnStatusTimer(wxTimerEvent&) {
    SetStatus("Ready", Theme::TXT_SECONDARY);
}