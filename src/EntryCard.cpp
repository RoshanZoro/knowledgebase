#include "EntryCard.h"
#include <wx/clipbrd.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/timer.h> // Essential for feedback
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

wxBEGIN_EVENT_TABLE(EntryCard, wxPanel)
wxEND_EVENT_TABLE()

static inline wxString U(const std::string& s) { return wxString::FromUTF8(s.c_str()); }

EntryCard::EntryCard(wxWindow* parent,
    const Entry& entry,
    const std::vector<std::string>& queryTerms,
    int cardIndex,
    int totalCards)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxBORDER_NONE | wxTAB_TRAVERSAL),
    m_entry(entry),
    m_terms(queryTerms)
{
    (void)cardIndex; (void)totalCards;
    SetBackgroundColour(Theme::BG_APP);
    BuildUI();
}

void EntryCard::SetHighlightTerms(const std::vector<std::string>& terms) {
    m_terms = terms;
}

wxColour EntryCard::HeaderColour() const {
    if (m_entry.type == "command") return Theme::BG_CARD_HDR_CMD;
    if (m_entry.type == "steps")   return Theme::BG_CARD_HDR_STEP;
    return Theme::BG_CARD_HDR_PROSE;
}
wxColour EntryCard::TagBgColour()  const {
    if (m_entry.type == "command") return Theme::BG_TAG_CMD;
    if (m_entry.type == "steps")   return Theme::BG_TAG_STEP;
    return Theme::BG_TAG_PROSE;
}
wxColour EntryCard::TagTxtColour() const {
    if (m_entry.type == "command") return Theme::TAG_CMD_TXT;
    if (m_entry.type == "steps")   return Theme::TAG_STEP_TXT;
    return Theme::TAG_PROSE_TXT;
}

bool EntryCard::IsCommandLine(const std::string& line) const {
    if (line.empty()) return false;
    if (m_entry.type != "command" && m_entry.type != "steps") return false;
    char c = line[0];
    if (std::isupper((unsigned char)c) && line.size() > 2) {
        if (line.back() == '.' && line.size() > 40) return false;
    }
    static const std::vector<std::string> cmdPrefixes = {
        "sudo","dnf","rpm","systemctl","firewall-cmd","useradd","usermod",
        "groupadd","chmod","chown","passwd","chage","ip","ping","ssh",
        "show","conf","int","router","switch","enable","no ","vlan",
        "access-list","ip route","ip nat","crypto","interface","ospf",
        "postfix","dovecot","openssl","certutil","netsh","powershell",
        "New-","Get-","Set-","Add-","Remove-","Import-","Export-",
        "mdadm","lvcreate","vgcreate","pvcreate","lvextend","mount",
        "easy-rsa","openvpn","wg ","certbot","semanage","restorecon",
    };
    for (auto& pfx : cmdPrefixes) {
        if (line.substr(0, pfx.size()) == pfx) return true;
    }
    if (c == '/' || c == '.' || c == '$' || c == '#') return true;
    if (line.find('.') == std::string::npos && line.size() < 120) return true;
    return false;
}

bool EntryCard::IsComment(const std::string& line) const {
    return !line.empty() && (line[0] == '#' || line[0] == '/');
}

bool EntryCard::IsStepLine(const std::string& line, int& outNum, std::string& outText) const {
    std::regex re(R"(^(\d+)\.\s+(.+)$)");
    std::smatch m;
    if (std::regex_match(line, m, re)) {
        outNum = std::stoi(m[1].str());
        outText = m[2].str();
        return true;
    }
    return false;
}

bool EntryCard::IsKeywordLine(const std::string& line) const {
    if (line.size() < 4) return false;
    return line.substr(0, 5) == "NOTE:" ||
        line.substr(0, 4) == "TIP:" ||
        line.substr(0, 8) == "WARNING:";
}

std::vector<std::string> EntryCard::ExtractCommandLines() const {
    std::vector<std::string> cmds;
    std::istringstream ss(m_entry.content);
    std::string line;
    while (std::getline(ss, line)) {
        std::string trimmed = line;
        while (!trimmed.empty() && std::isspace((unsigned char)trimmed.front())) trimmed.erase(trimmed.begin());
        while (!trimmed.empty() && std::isspace((unsigned char)trimmed.back()))  trimmed.pop_back();
        if (!trimmed.empty() && IsCommandLine(trimmed) && !IsComment(trimmed))
            cmds.push_back(trimmed);
    }
    return cmds;
}

void EntryCard::CopyToClipboard(const std::string& text) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(U(text)));
        wxTheClipboard->Close();
    }
}

void EntryCard::CopyAllCommands() {
    auto cmds = ExtractCommandLines();
    std::string joined;
    for (auto& c : cmds) joined += c + "\n";
    if (!joined.empty()) CopyToClipboard(joined);
}

void EntryCard::AddHighlightedText(wxSizer* sizer, wxWindow* parent, const std::string& text, const wxFont& font, const wxColour& baseColour) {
    if (m_terms.empty() || text.empty()) {
        auto* lbl = new wxStaticText(parent, wxID_ANY, U(text));
        lbl->SetFont(font);
        lbl->SetForegroundColour(baseColour);
        lbl->SetBackgroundColour(parent->GetBackgroundColour());
        sizer->Add(lbl, 0, wxALIGN_CENTER_VERTICAL);
        return;
    }

    std::string pattern;
    for (size_t i = 0; i < m_terms.size(); ++i) {
        if (i > 0) pattern += "|";
        for (char c : m_terms[i]) {
            if (std::string(".*+?^${}()|[]\\").find(c) != std::string::npos) pattern += '\\';
            pattern += c;
        }
    }

    std::vector<std::pair<std::string, bool>> spans;
    try {
        std::regex re(pattern, std::regex::icase);
        std::sregex_iterator it(text.begin(), text.end(), re), end;
        size_t pos = 0;
        for (; it != end; ++it) {
            auto& m = *it;
            if (m.position() > (int)pos) spans.push_back({ text.substr(pos, m.position() - pos), false });
            spans.push_back({ m.str(), true });
            pos = m.position() + m.length();
        }
        if (pos < text.size()) spans.push_back({ text.substr(pos), false });
    }
    catch (...) { spans.push_back({ text, false }); }

    for (auto& [span, isMatch] : spans) {
        if (span.empty()) continue;
        auto* lbl = new wxStaticText(parent, wxID_ANY, U(span));
        lbl->SetFont(isMatch ? wxFont(font.GetPointSize(), font.GetFamily(), font.GetStyle(), wxFONTWEIGHT_BOLD) : font);
        lbl->SetForegroundColour(isMatch ? Theme::ACCENT : baseColour);
        lbl->SetBackgroundColour(parent->GetBackgroundColour());
        sizer->Add(lbl, 0, wxALIGN_CENTER_VERTICAL);
    }
}

wxWindow* EntryCard::BuildCommandLine(wxWindow* parent, const std::string& line, bool isComment) {
    auto* row = new wxPanel(parent, wxID_ANY);
    row->SetBackgroundColour(isComment ? Theme::BG_CARD : Theme::SYN_CMD_BG);
    auto* rowSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* accent = new wxPanel(row, wxID_ANY, wxDefaultPosition, wxSize(3, -1));
    accent->SetBackgroundColour(isComment ? Theme::SYN_COMMENT : Theme::SYN_CMD);
    rowSizer->Add(accent, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);

    auto* textPanel = new wxPanel(row, wxID_ANY);
    textPanel->SetBackgroundColour(row->GetBackgroundColour());
    auto* textSizer = new wxBoxSizer(wxHORIZONTAL);
    wxFont monoFont = Theme::FontMono(10);
    wxColour txtCol = isComment ? Theme::SYN_COMMENT : Theme::SYN_CMD;

    if (!isComment && line.find(" -") != std::string::npos) {
        std::istringstream ss(line); std::string token; bool firstToken = true;
        while (ss >> token) {
            auto* lbl = new wxStaticText(textPanel, wxID_ANY, firstToken ? U(token) : U(" " + token));
            lbl->SetFont(monoFont);
            lbl->SetForegroundColour(!firstToken && token[0] == '-' ? Theme::SYN_FLAG : txtCol);
            textSizer->Add(lbl, 0, wxALIGN_CENTER_VERTICAL);
            firstToken = false;
        }
    }
    else { AddHighlightedText(textSizer, textPanel, line, monoFont, txtCol); }

    textPanel->SetSizer(textSizer);
    rowSizer->AddSpacer(8);
    rowSizer->Add(textPanel, 1, wxEXPAND | wxTOP | wxBOTTOM, 5);

    if (!isComment) {
        auto* copyBtn = new wxButton(row, wxID_ANY, "Copy", wxDefaultPosition, wxSize(50, 22), wxBORDER_NONE);
        copyBtn->SetFont(Theme::FontUI(8));
        copyBtn->SetForegroundColour(Theme::ACCENT);
        copyBtn->SetBackgroundColour(Theme::BG_APP);

        copyBtn->Bind(wxEVT_BUTTON, [this, line, copyBtn](wxCommandEvent&) {
            this->CopyToClipboard(line);
            copyBtn->SetLabel(wxT("✓"));

            // Standard timer logic for label reset
            wxTimer* timer = new wxTimer();
            copyBtn->Bind(wxEVT_TIMER, [copyBtn, timer](wxTimerEvent&) {
                if (copyBtn) copyBtn->SetLabel("Copy");
                timer->Stop();
                delete timer;
                }, timer->GetId());
            timer->StartOnce(1000);
            });
        rowSizer->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    }
    row->SetSizer(rowSizer);
    return row;
}

wxWindow* EntryCard::BuildStepLine(wxWindow* parent, int num, const std::string& text) {
    auto* row = new wxPanel(parent, wxID_ANY);
    row->SetBackgroundColour(Theme::BG_CARD);
    auto* s = new wxBoxSizer(wxHORIZONTAL);
    auto* numLbl = new wxStaticText(row, wxID_ANY, wxString::Format("%d.", num), wxDefaultPosition, wxSize(28, -1));
    numLbl->SetFont(Theme::FontUI(10, true));
    numLbl->SetForegroundColour(Theme::SYN_STEP_NUM);
    auto* tp = new wxPanel(row, wxID_ANY); tp->SetBackgroundColour(Theme::BG_CARD);
    auto* ts = new wxBoxSizer(wxHORIZONTAL); AddHighlightedText(ts, tp, text, Theme::FontUI(10), Theme::TXT_NORMAL);
    tp->SetSizer(ts);
    s->Add(numLbl, 0, wxALIGN_TOP | wxTOP, 2); s->AddSpacer(4); s->Add(tp, 1, wxEXPAND);
    row->SetSizer(s); return row;
}

wxWindow* EntryCard::BuildProseLine(wxWindow* parent, const std::string& line) {
    auto* row = new wxPanel(parent, wxID_ANY); row->SetBackgroundColour(Theme::BG_CARD);
    auto* s = new wxBoxSizer(wxHORIZONTAL);
    auto* tp = new wxPanel(row, wxID_ANY); tp->SetBackgroundColour(Theme::BG_CARD);
    auto* ts = new wxBoxSizer(wxHORIZONTAL); AddHighlightedText(ts, tp, line, Theme::FontUI(10), IsKeywordLine(line) ? Theme::SYN_KEYWORD : Theme::TXT_NORMAL);
    tp->SetSizer(ts); s->Add(tp, 1, wxEXPAND); row->SetSizer(s); return row;
}

wxWindow* EntryCard::BuildContentArea(wxWindow* parent) {
    auto* content = new wxPanel(parent, wxID_ANY);
    content->SetBackgroundColour(Theme::BG_CARD);
    auto* colSizer = new wxBoxSizer(wxVERTICAL);
    std::istringstream ss(m_entry.content); std::string line; bool prevEmpty = false;
    while (std::getline(ss, line)) {
        while (!line.empty() && std::isspace((unsigned char)line.back())) line.pop_back();
        if (line.empty()) { if (!prevEmpty) colSizer->AddSpacer(4); prevEmpty = true; continue; }
        prevEmpty = false;
        int sNum; std::string sTxt;
        if (IsStepLine(line, sNum, sTxt)) colSizer->Add(BuildStepLine(content, sNum, sTxt), 0, wxEXPAND | wxBOTTOM, 3);
        else if (IsComment(line)) colSizer->Add(BuildCommandLine(content, line, true), 0, wxEXPAND | wxBOTTOM, 2);
        else if (IsCommandLine(line)) colSizer->Add(BuildCommandLine(content, line, false), 0, wxEXPAND | wxBOTTOM, 2);
        else colSizer->Add(BuildProseLine(content, line), 0, wxEXPAND | wxBOTTOM, 2);
    }
    content->SetSizer(colSizer); return content;
}

void EntryCard::BuildUI() {
    auto* outerSizer = new wxBoxSizer(wxVERTICAL);
    auto* card = new wxPanel(this, wxID_ANY);
    card->SetBackgroundColour(Theme::BG_CARD);
    auto* cardSizer = new wxBoxSizer(wxVERTICAL);

    auto* stripe = new wxPanel(card, wxID_ANY, wxDefaultPosition, wxSize(-1, Theme::CARD_HDR_H));
    stripe->SetBackgroundColour(HeaderColour());
    cardSizer->Add(stripe, 0, wxEXPAND);

    auto* header = new wxPanel(card, wxID_ANY);
    auto* hSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* badge = new wxStaticText(header, wxID_ANY, U(m_entry.type).Upper());
    badge->SetFont(Theme::FontUI(8, true)); badge->SetForegroundColour(TagTxtColour()); badge->SetBackgroundColour(TagBgColour());

    std::string crumb = m_entry.section + (m_entry.subsection.empty() ? "" : "  ›  " + m_entry.subsection);
    auto* crumbLbl = new wxStaticText(header, wxID_ANY, U(crumb));
    crumbLbl->SetFont(Theme::FontUI(9)); crumbLbl->SetForegroundColour(Theme::TXT_SECTION);

    hSizer->AddSpacer(Theme::CARD_PAD); hSizer->Add(badge, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
    hSizer->AddSpacer(10); hSizer->Add(crumbLbl, 0, wxALIGN_CENTER_VERTICAL);
    hSizer->AddStretchSpacer(1);

    if (m_entry.type == "command" || m_entry.type == "steps") {
        auto* copyAll = new wxButton(header, wxID_ANY, "Copy All", wxDefaultPosition, wxSize(70, 22), wxBORDER_NONE);
        copyAll->SetFont(Theme::FontUI(8, true));
        copyAll->SetForegroundColour(Theme::ACCENT);

        copyAll->Bind(wxEVT_BUTTON, [this, copyAll](wxCommandEvent&) {
            this->CopyAllCommands();
            copyAll->SetLabel("Done");

            wxTimer* timer = new wxTimer();
            copyAll->Bind(wxEVT_TIMER, [copyAll, timer](wxTimerEvent&) {
                if (copyAll) copyAll->SetLabel("Copy All");
                timer->Stop();
                delete timer;
                }, timer->GetId());
            timer->StartOnce(1000);
            });
        hSizer->Add(copyAll, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    }

    auto* srcLbl = new wxStaticText(header, wxID_ANY, U(m_entry.source));
    srcLbl->SetFont(Theme::FontUI(8)); srcLbl->SetForegroundColour(Theme::TXT_SECONDARY);
    hSizer->Add(srcLbl, 0, wxALIGN_CENTER_VERTICAL); hSizer->AddSpacer(Theme::CARD_PAD);

    header->SetSizer(hSizer); cardSizer->Add(header, 0, wxEXPAND);
    cardSizer->Add(new wxStaticLine(card, wxID_ANY), 0, wxEXPAND);
    cardSizer->Add(BuildContentArea(card), 0, wxEXPAND | wxALL, Theme::CARD_PAD);

    card->SetSizer(cardSizer);
    outerSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, Theme::CARD_GAP);
    SetSizer(outerSizer); Layout();
}