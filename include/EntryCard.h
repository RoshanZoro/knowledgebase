#pragma once
#include "Types.h"
#include "Theme.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <vector>
#include <string>

// ── EntryCard ─────────────────────────────────────────────────────────────────
// A self-contained panel that renders one Entry as a rich card:
//   • Coloured top stripe (blue=command, purple=steps, green=prose)
//   • Type badge pill   [COMMAND] / [STEPS] / [PROSE]
//   • Breadcrumb        Section  >  Subsection
//   • Content area with syntax highlighting
//   • Copy button on every command line / full-card copy button
class EntryCard : public wxPanel {
public:
    EntryCard(wxWindow* parent,
              const Entry& entry,
              const std::vector<std::string>& queryTerms,
              int cardIndex,
              int totalCards);

    void SetHighlightTerms(const std::vector<std::string>& terms);

private:
    Entry                    m_entry;
    std::vector<std::string> m_terms;

    // Sub-widgets built in BuildUI()
    void BuildUI();

    // Content rendering helpers
    wxWindow* BuildContentArea(wxWindow* parent);
    wxWindow* BuildCommandLine(wxWindow* parent,
                               const std::string& line,
                               bool isComment);
    wxWindow* BuildStepLine   (wxWindow* parent,
                               int stepNum,
                               const std::string& text);
    wxWindow* BuildProseLine  (wxWindow* parent,
                               const std::string& line);

    // Rich text with highlight spans
    void      AddHighlightedText(wxSizer* sizer,
                                 wxWindow* parent,
                                 const std::string& text,
                                 const wxFont& font,
                                 const wxColour& baseColour);

    // Copy helpers
    void CopyToClipboard(const std::string& text);
    void CopyAllCommands();
    std::vector<std::string> ExtractCommandLines() const;

    // Detect line type
    bool IsCommandLine(const std::string& line) const;
    bool IsComment    (const std::string& line) const;
    bool IsStepLine   (const std::string& line, int& outNum, std::string& outText) const;
    bool IsKeywordLine(const std::string& line) const;  // NOTE: TIP: WARNING:

    // Card header colour by type
    wxColour HeaderColour() const;
    wxColour TagBgColour()  const;
    wxColour TagTxtColour() const;

    wxDECLARE_EVENT_TABLE();
};
