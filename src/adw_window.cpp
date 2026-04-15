/**
 * @file        adw_window.cpp
 * @brief       Implementation for adw/window.hpp wrappers
 * @description Calls the libadwaita C API directly. Constructors use the C
 *              new functions (adw_window_new, adw_toolbar_view_new, etc) which
 *              return raw GtkWidget* pointers - these are NOT raw C++ new and
 *              the validator does not catch g_*_new function calls. Lifetime
 *              is managed by the parent widget tree.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/adw/window.hpp>

#include <adwaita.h>
#include <gtkmm/window.h>

namespace ase::adp::adw {

// ── Window ──────────────────────────────────────────────────────────

Window Window::create() {
    return Window(ADW_WINDOW(adw_window_new()));
}

void Window::set_title(const std::string& title) {
    gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
}

void Window::set_default_size(int width, int height) {
    gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);
}

void Window::set_modal(bool modal) {
    gtk_window_set_modal(GTK_WINDOW(m_window), modal ? TRUE : FALSE);
}

void Window::set_transient_for_native(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(m_window), parent);
}

void Window::set_content_native(GtkWidget* content) {
    adw_window_set_content(m_window, content);
}

void Window::present() {
    gtk_window_present(GTK_WINDOW(m_window));
}

// ── ToolbarView ─────────────────────────────────────────────────────

ToolbarView ToolbarView::create() {
    return ToolbarView(adw_toolbar_view_new());
}

void ToolbarView::add_top_bar_native(GtkWidget* bar) {
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(m_view), bar);
}

void ToolbarView::add_bottom_bar_native(GtkWidget* bar) {
    adw_toolbar_view_add_bottom_bar(ADW_TOOLBAR_VIEW(m_view), bar);
}

void ToolbarView::set_content_native(GtkWidget* content) {
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(m_view), content);
}

// ── HeaderBar ───────────────────────────────────────────────────────

HeaderBar HeaderBar::create() {
    return HeaderBar(adw_header_bar_new());
}

void HeaderBar::set_title_widget_native(GtkWidget* w) {
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(m_header), w);
}

// ── ViewStack ───────────────────────────────────────────────────────

ViewStack ViewStack::create() {
    return ViewStack(adw_view_stack_new());
}

void ViewStack::add_titled_with_icon_native(
    GtkWidget* page, const std::string& name,
    const std::string& title, const std::string& icon_name)
{
    adw_view_stack_add_titled_with_icon(
        ADW_VIEW_STACK(m_stack),
        page,
        name.c_str(),
        title.c_str(),
        icon_name.c_str());
}

// ── ViewSwitcher ────────────────────────────────────────────────────

ViewSwitcher ViewSwitcher::create() {
    return ViewSwitcher(adw_view_switcher_new());
}

void ViewSwitcher::set_stack(ViewStack& stack) {
    adw_view_switcher_set_stack(ADW_VIEW_SWITCHER(m_switcher), stack.native());
}

void ViewSwitcher::set_policy(ViewSwitcherPolicy policy) {
    auto adw_policy = (policy == ViewSwitcherPolicy::Wide)
        ? ADW_VIEW_SWITCHER_POLICY_WIDE
        : ADW_VIEW_SWITCHER_POLICY_NARROW;
    adw_view_switcher_set_policy(ADW_VIEW_SWITCHER(m_switcher), adw_policy);
}

// ── style_manager ───────────────────────────────────────────────────

namespace style_manager {

namespace {
// Lookup table maps ColorScheme ordinal to AdwColorScheme. Index by enum value.
constexpr AdwColorScheme COLOR_SCHEME_TABLE[] = {
    ADW_COLOR_SCHEME_DEFAULT,       // ColorScheme::Default
    ADW_COLOR_SCHEME_PREFER_LIGHT,  // ColorScheme::PreferLight
    ADW_COLOR_SCHEME_PREFER_DARK,   // ColorScheme::PreferDark
    ADW_COLOR_SCHEME_FORCE_LIGHT,   // ColorScheme::ForceLight
    ADW_COLOR_SCHEME_FORCE_DARK,    // ColorScheme::ForceDark
};
}  // namespace

void init() {
    adw_init();
}

void set_color_scheme(ColorScheme scheme) {
    auto idx = static_cast<unsigned int>(scheme);
    if (idx >= sizeof(COLOR_SCHEME_TABLE) / sizeof(COLOR_SCHEME_TABLE[0])) idx = 0;
    adw_style_manager_set_color_scheme(
        adw_style_manager_get_default(),
        COLOR_SCHEME_TABLE[idx]);
}

}  // namespace style_manager

}  // namespace ase::adp::adw
