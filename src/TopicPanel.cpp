#include "TopicPanel.h"
#include <wx/dcbuffer.h>
#include <wx/sizer.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  TopicRow
// ═══════════════════════════════════════════════════════════════════════════════

wxBEGIN_EVENT_TABLE(TopicPanel::TopicRow, wxPanel)
    EVT_PAINT        (TopicPanel::TopicRow::OnPaint)
    EVT_ENTER_WINDOW (TopicPanel::TopicRow::OnEnter)
    EVT_LEAVE_WINDOW (TopicPanel::TopicRow::OnLeave)
    EVT_LEFT_DOWN    (TopicPanel::TopicRow::OnClick)
wxEND_EVENT_TABLE()

TopicPanel::TopicRow::TopicRow(wxWindow* parent, const Topic& topic, TopicPanel* owner)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 34), wxBORDER_NONE)
    , m_topic(topic), m_owner(owner)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetCursor(wxCursor(wxCURSOR_HAND));
    SetMinSize(wxSize(-1, 34));

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_label = new wxStaticText(this, wxID_ANY,
                               wxString::FromUTF8(topic.display.c_str()),
                               wxDefaultPosition, wxDefaultSize,
                               wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_END);
    m_label->SetFont(Theme::FontUI(10));
    m_label->SetForegroundColour(Theme::TXT_SIDEBAR);
    m_label->SetBackgroundColour(Theme::BG_SIDEBAR);
    m_label->SetCursor(wxCursor(wxCURSOR_HAND));
    m_label->Bind(wxEVT_LEFT_DOWN, &TopicRow::OnClick, this);

    sizer->AddSpacer(20);
    sizer->Add(m_label, 1, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 6);
    sizer->AddSpacer(8);
    SetSizer(sizer);
}

void TopicPanel::TopicRow::OnPaint(wxPaintEvent&) {
    wxBufferedPaintDC dc(this);
    wxRect r = GetClientRect();

    wxColour bg = m_selected ? Theme::BG_SIDEBAR_S
                : m_hovered  ? Theme::BG_SIDEBAR_H
                :              Theme::BG_SIDEBAR;
    dc.SetBackground(wxBrush(bg));
    dc.Clear();

    if (m_selected) {
        dc.SetPen(wxPen(*wxWHITE, 3));
        dc.DrawLine(0, 4, 0, r.GetHeight() - 4);
    }

    wxColour fg = m_selected ? Theme::TXT_SIDEBAR_S
                : m_hovered  ? *wxWHITE
                :              Theme::TXT_SIDEBAR;
    m_label->SetForegroundColour(fg);
    m_label->SetBackgroundColour(bg);
}

void TopicPanel::TopicRow::OnEnter(wxMouseEvent& e) { m_hovered = true;  Refresh(); e.Skip(); }
void TopicPanel::TopicRow::OnLeave(wxMouseEvent& e) { m_hovered = false; Refresh(); e.Skip(); }
void TopicPanel::TopicRow::OnClick(wxMouseEvent&) {
    m_owner->ClearSelection();
    SetSelected(true);
    if (m_owner->m_callback) m_owner->m_callback(m_topic);
}
void TopicPanel::TopicRow::SetSelected(bool sel) { m_selected = sel; Refresh(); }

// ═══════════════════════════════════════════════════════════════════════════════
//  TopicPanel
// ═══════════════════════════════════════════════════════════════════════════════

TopicPanel::TopicPanel(wxWindow* parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxVSCROLL | wxBORDER_NONE)
{
    SetBackgroundColour(Theme::BG_SIDEBAR);
    SetScrollRate(0, 10);

    m_sizer = new wxBoxSizer(wxVERTICAL);

    auto* logoPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 60));
    logoPanel->SetBackgroundColour(Theme::BG_TOPBAR);
    auto* logoSizer = new wxBoxSizer(wxVERTICAL);

    auto* appName = new wxStaticText(logoPanel, wxID_ANY, wxT("KB Search"));
    appName->SetFont(Theme::FontTitle(14));
    appName->SetForegroundColour(*wxWHITE);
    appName->SetBackgroundColour(Theme::BG_TOPBAR);

    auto* tagline = new wxStaticText(logoPanel, wxID_ANY, wxT("Knowledge Base"));
    tagline->SetFont(Theme::FontUI(8));
    tagline->SetForegroundColour(wxColour(140, 150, 170));
    tagline->SetBackgroundColour(Theme::BG_TOPBAR);

    logoSizer->AddStretchSpacer(1);
    logoSizer->Add(appName, 0, wxLEFT, 16);
    logoSizer->Add(tagline, 0, wxLEFT, 16);
    logoSizer->AddStretchSpacer(1);
    logoPanel->SetSizer(logoSizer);

    m_sizer->Add(logoPanel, 0, wxEXPAND);
    m_sizer->AddSpacer(8);
    SetSizer(m_sizer);
}

void TopicPanel::LoadCategories(const std::vector<Category>& cats) {
    m_categories = cats;
    m_rows.clear();

    while (m_sizer->GetItemCount() > 2) {
        size_t idx = m_sizer->GetItemCount() - 1;
        wxSizerItem* item = m_sizer->GetItem(idx);
        wxWindow* win = item ? item->GetWindow() : nullptr;
        m_sizer->Detach(idx);
        if (win) win->Destroy();
    }

    BuildList();
    Layout();
    FitInside();
}

void TopicPanel::BuildList() {
    for (auto& cat : m_categories) {
        AddCategoryHeader(cat.name);
        for (auto& topic : cat.topics)
            AddTopicRow(topic);
        m_sizer->AddSpacer(4);
    }
}

void TopicPanel::AddCategoryHeader(const std::string& name) {
    auto* hdr = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 28));
    hdr->SetBackgroundColour(Theme::BG_SIDEBAR);
    auto* s = new wxBoxSizer(wxHORIZONTAL);
    auto* lbl = new wxStaticText(hdr, wxID_ANY,
                                  wxString::FromUTF8(name.c_str()).Upper());
    lbl->SetFont(Theme::FontUI(8, true));
    lbl->SetForegroundColour(wxColour(100, 110, 130));
    lbl->SetBackgroundColour(Theme::BG_SIDEBAR);
    s->AddSpacer(14);
    s->Add(lbl, 0, wxALIGN_CENTER_VERTICAL);
    hdr->SetSizer(s);
    m_sizer->Add(hdr, 0, wxEXPAND);
}

void TopicPanel::AddTopicRow(const Topic& topic) {
    auto* row = new TopicRow(this, topic, this);
    m_rows.push_back(row);
    m_sizer->Add(row, 0, wxEXPAND);
}

void TopicPanel::ClearSelection() {
    for (auto* r : m_rows) r->SetSelected(false);
}