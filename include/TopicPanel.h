#pragma once
#include "Types.h"
#include "Theme.h"
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <vector>
#include <functional>

class TopicPanel : public wxScrolledWindow {
public:
    using TopicCallback = std::function<void(const Topic&)>;

    explicit TopicPanel(wxWindow* parent);

    void LoadCategories(const std::vector<Category>& cats);
    void SetCallback(TopicCallback cb) { m_callback = cb; }
    void ClearSelection();

private:
    TopicCallback         m_callback;
    std::vector<Category> m_categories;
    wxBoxSizer*           m_sizer      = nullptr;
    wxWindow*             m_selected   = nullptr;
    int                   m_selectedId = -1;

    void BuildList();
    void AddCategoryHeader(const std::string& name);
    void AddTopicRow(const Topic& topic);

    class TopicRow : public wxPanel {
    public:
        TopicRow(wxWindow* parent, const Topic& topic, TopicPanel* owner);
        void SetSelected(bool sel);
        const Topic& GetTopic() const { return m_topic; }
    private:
        Topic         m_topic;
        TopicPanel*   m_owner;
        bool          m_selected = false;
        bool          m_hovered  = false;
        wxStaticText* m_label    = nullptr;

        void OnPaint (wxPaintEvent& e);
        void OnEnter (wxMouseEvent& e);
        void OnLeave (wxMouseEvent& e);
        void OnClick (wxMouseEvent& e);
        void RefreshStyle();
        wxDECLARE_EVENT_TABLE();
    };

    std::vector<TopicRow*> m_rows;
};