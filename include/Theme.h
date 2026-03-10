#pragma once
#include <wx/wx.h>
#include <wx/font.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  KB Theme  —  Light / Clean / Professional
//  All colours, fonts and metrics centralised here.
// ═══════════════════════════════════════════════════════════════════════════════

namespace Theme {

// ── Base colours ──────────────────────────────────────────────────────────────
inline const wxColour BG_APP       { 245, 246, 250 };  // app background
inline const wxColour BG_PANEL     { 255, 255, 255 };  // card / panel white
inline const wxColour BG_SIDEBAR   {  36,  41,  51 };  // left sidebar dark slate
inline const wxColour BG_SIDEBAR_H {  49,  55,  68 };  // sidebar hover
inline const wxColour BG_SIDEBAR_S {  22, 119, 255 };  // sidebar selected (accent blue)
inline const wxColour BG_TOPBAR    {  22,  27,  38 };  // top bar near-black
inline const wxColour BG_SEARCH    { 255, 255, 255 };  // search box
inline const wxColour BG_CARD      { 255, 255, 255 };  // result card
inline const wxColour BG_CARD_HDR_CMD  {  22, 119, 255 };  // command header stripe
inline const wxColour BG_CARD_HDR_STEP { 124,  77, 255 };  // steps header stripe
inline const wxColour BG_CARD_HDR_PROSE{  82, 196,  26 };  // prose header stripe
inline const wxColour BG_TAG_CMD   { 230, 241, 255 };  // command type pill bg
inline const wxColour BG_TAG_STEP  { 243, 236, 255 };  // steps type pill bg
inline const wxColour BG_TAG_PROSE { 236, 248, 230 };  // prose type pill bg
inline const wxColour BG_COPY_BTN  { 240, 245, 255 };  // copy button bg
inline const wxColour BG_COPY_BTN_H{ 214, 229, 255 };  // copy button hover

inline const wxColour BORDER       { 220, 223, 230 };  // card border
inline const wxColour BORDER_FOCUS {  22, 119, 255 };  // search focus ring
inline const wxColour DIVIDER      { 238, 240, 245 };  // section divider

// ── Text colours ──────────────────────────────────────────────────────────────
inline const wxColour TXT_PRIMARY  {  15,  17,  21 };  // headings / labels
inline const wxColour TXT_SECONDARY{  88,  96, 112 };  // muted / meta
inline const wxColour TXT_SIDEBAR  { 179, 187, 204 };  // sidebar text
inline const wxColour TXT_SIDEBAR_S{ 255, 255, 255 };  // sidebar selected
inline const wxColour TXT_ACCENT   {  22, 119, 255 };  // blue accent
inline const wxColour TXT_SECTION  {  88,  96, 112 };  // section breadcrumb

// ── Syntax / content colours ──────────────────────────────────────────────────
inline const wxColour SYN_CMD      {  22, 119, 255 };  // CLI command text (blue)
inline const wxColour SYN_CMD_BG   { 240, 246, 255 };  // CLI command line bg tint
inline const wxColour SYN_FLAG     { 214,  90,  49 };  // flag/option text (orange)
inline const wxColour SYN_COMMENT  { 108, 117, 125 };  // # comment text
inline const wxColour SYN_STEP_NUM { 124,  77, 255 };  // numbered step prefix
inline const wxColour SYN_KEYWORD  { 10,  130,  80 };  // keywords NOTE/TIP/WARNING
inline const wxColour SYN_HIGHLIGHT{ 255, 235,  59 };  // query term highlight bg
inline const wxColour TXT_NORMAL   {  30,  34,  42 };  // regular content text

// ── Accent / UI colours ───────────────────────────────────────────────────────
inline const wxColour ACCENT       {  22, 119, 255 };  // primary blue
inline const wxColour ACCENT_LIGHT { 240, 246, 255 };  // very light blue tint
inline const wxColour SUCCESS      {  82, 196,  26 };  // green
inline const wxColour WARNING      { 250, 173,  20 };  // amber
inline const wxColour PURPLE       { 124,  77, 255 };  // purple (steps)

// ── Tag pill text colours ─────────────────────────────────────────────────────
inline const wxColour TAG_CMD_TXT  {  22, 119, 255 };
inline const wxColour TAG_STEP_TXT { 124,  77, 255 };
inline const wxColour TAG_PROSE_TXT{  56, 158,  13 };

// ── Metrics ───────────────────────────────────────────────────────────────────
constexpr int SIDEBAR_W       = 230;
constexpr int TOPBAR_H        = 56;
constexpr int SEARCH_H        = 44;
constexpr int CARD_RADIUS     = 8;
constexpr int CARD_PAD        = 16;
constexpr int CARD_GAP        = 10;
constexpr int CARD_HDR_H      = 6;   // coloured top stripe height
constexpr int COPY_BTN_W      = 72;
constexpr int COPY_BTN_H      = 26;

// ── Font helpers ──────────────────────────────────────────────────────────────
inline wxFont FontUI(int pts = 10, bool bold = false) {
    return wxFont(pts, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                  bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, false,
                  wxT("Segoe UI"));
}
inline wxFont FontMono(int pts = 10) {
    return wxFont(pts, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_NORMAL, false, wxT("Consolas"));
}
inline wxFont FontTitle(int pts = 12) {
    return wxFont(pts, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_BOLD, false, wxT("Segoe UI"));
}

} // namespace Theme
