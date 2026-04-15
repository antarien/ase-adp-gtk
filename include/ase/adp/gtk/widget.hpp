#pragma once

/**
 * @file        widget.hpp
 * @brief       Box, Button, Label, ScrolledWindow, HeaderBar, SearchEntry wrappers
 * @description ASE-native wrappers around the most commonly used gtkmm-4 widget
 *              classes. Each wrapper composes a Gtk::Foo* internally and exposes
 *              only ASE-native types. Wrappers do NOT share an inheritance
 *              hierarchy - inheritance is only allowed against gtkmm bases here,
 *              not against other ase::adp::gtk wrappers. Common widget operations are
 *              delegated to free helper functions in ase::adp::gtk::detail.
 *
 *              Callbacks are template parameters (any callable: lambda, function
 *              pointer, member-function bind) - the validator forbids std::function
 *              even in the adapter, so templates carry the callable type inline.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <gtkmm/widget.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/searchentry.h>
#include <pangomm/layout.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/** Stacking orientation for Box and similar containers. */
enum class Orientation { Horizontal, Vertical };

namespace detail {

// Free helpers delegated to by every wrapper for common widget operations.
// Defined out-of-line in src/gtk_widget.cpp to keep gtkmm symbols out of the
// caller translation unit symbol table where possible.
void widget_set_margin_start(Gtk::Widget* w, int margin);
void widget_set_margin_end(Gtk::Widget* w, int margin);
void widget_set_margin_top(Gtk::Widget* w, int margin);
void widget_set_margin_bottom(Gtk::Widget* w, int margin);
void widget_set_margin_all(Gtk::Widget* w, int margin);
void widget_set_hexpand(Gtk::Widget* w, bool expand);
void widget_set_vexpand(Gtk::Widget* w, bool expand);
void widget_add_css_class(Gtk::Widget* w, const std::string& cls);
void widget_remove_css_class(Gtk::Widget* w, const std::string& cls);
void widget_set_visible(Gtk::Widget* w, bool visible);
bool widget_get_visible(const Gtk::Widget* w);
void widget_set_size_request(Gtk::Widget* w, int width, int height);
void widget_set_tooltip_text(Gtk::Widget* w, const std::string& text);
void widget_grab_focus(Gtk::Widget* w);

}  // namespace detail

/**
 * Box - horizontal or vertical container that arranges its children in a single
 * line. append() and prepend() take any wrapper that exposes native_widget().
 */
class Box {
public:
    static Box horizontal(int spacing = 0);
    static Box vertical(int spacing = 0);
    static Box create(Orientation orientation, int spacing = 0);

    template <typename W>
    void append(W& child) { append_native(child.native_widget()); }

    template <typename W>
    void prepend(W& child) { prepend_native(child.native_widget()); }

    template <typename W>
    void remove(W& child) { remove_native(child.native_widget()); }

    void remove_all_children();

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_box, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_box, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_box, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_box, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_box, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_box, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_box, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_box, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_box, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_box, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_box); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_box, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_box, t); }
    void grab_focus()             { detail::widget_grab_focus(m_box); }
    Gtk::Widget* native_widget() const noexcept { return m_box; }
    Gtk::Box* native() const noexcept { return m_box; }

private:
    explicit Box(Gtk::Box* b) : m_box(b) {}
    void append_native(Gtk::Widget* w);
    void prepend_native(Gtk::Widget* w);
    void remove_native(Gtk::Widget* w);
    Gtk::Box* m_box;
};

/**
 * Button - clickable button with text label or symbolic icon. Click events are
 * delivered via on_clicked(callable). The callable can be any invocable with
 * signature void() - lambdas with captures are supported via templating.
 */
class Button {
public:
    static Button create(const std::string& label);
    static Button create_from_icon(const std::string& icon_name);
    static Button create();

    void set_label(const std::string& label);
    void set_icon_name(const std::string& icon_name);

    template <typename Callback>
    void on_clicked(Callback&& callback) {
        connect_clicked_impl(sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_button, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_button, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_button, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_button, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_button, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_button, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_button, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_button, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_button, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_button, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_button); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_button, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_button, t); }
    void grab_focus()             { detail::widget_grab_focus(m_button); }
    Gtk::Widget* native_widget() const noexcept { return m_button; }
    Gtk::Button* native() const noexcept { return m_button; }

private:
    explicit Button(Gtk::Button* b) : m_button(b) {}
    void connect_clicked_impl(sigc::slot<void()> slot);
    Gtk::Button* m_button;
};

/**
 * Label - displays a plain text or Pango-marked-up string.
 */
class Label {
public:
    static Label create(const std::string& text = "");

    void set_text(const std::string& text);
    void set_markup(const std::string& markup);
    std::string get_text() const;

    /** Horizontal alignment in [0.0, 1.0]: 0.0 = left, 0.5 = center, 1.0 = right. */
    void set_xalign(float align);

    /** Truncate with an end ellipsis when the label exceeds its allocated width. */
    void enable_ellipsize_end();

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_label, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_label, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_label, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_label, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_label, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_label, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_label, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_label, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_label, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_label, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_label); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_label, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_label, t); }
    void grab_focus()             { detail::widget_grab_focus(m_label); }
    Gtk::Widget* native_widget() const noexcept { return m_label; }
    Gtk::Label* native() const noexcept { return m_label; }

private:
    explicit Label(Gtk::Label* l) : m_label(l) {}
    Gtk::Label* m_label;
};

/**
 * ScrolledWindow - container that adds horizontal/vertical scrollbars to a single
 * child widget when its natural size exceeds the visible area.
 */
class ScrolledWindow {
public:
    static ScrolledWindow create();

    template <typename W>
    void set_child(W& child) { set_child_native(child.native_widget()); }

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_sw, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_sw, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_sw, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_sw, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_sw, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_sw, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_sw, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_sw, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_sw, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_sw, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_sw); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_sw, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_sw, t); }
    void grab_focus()             { detail::widget_grab_focus(m_sw); }
    Gtk::Widget* native_widget() const noexcept { return m_sw; }
    Gtk::ScrolledWindow* native() const noexcept { return m_sw; }

private:
    explicit ScrolledWindow(Gtk::ScrolledWindow* sw) : m_sw(sw) {}
    void set_child_native(Gtk::Widget* w);
    Gtk::ScrolledWindow* m_sw;
};

/**
 * HeaderBar - the title bar at the top of a window. Holds a centered title widget
 * and packs additional widgets to the start (left) or end (right).
 */
class HeaderBar {
public:
    static HeaderBar create();

    template <typename W>
    void pack_start(W& w) { pack_start_native(w.native_widget()); }

    template <typename W>
    void pack_end(W& w) { pack_end_native(w.native_widget()); }

    template <typename W>
    void set_title_widget(W& w) { set_title_widget_native(w.native_widget()); }

    void set_show_title_buttons(bool show);

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_header, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_header, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_header, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_header, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_header, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_header, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_header, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_header, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_header, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_header, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_header); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_header, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_header, t); }
    void grab_focus()             { detail::widget_grab_focus(m_header); }
    Gtk::Widget* native_widget() const noexcept { return m_header; }
    Gtk::HeaderBar* native() const noexcept { return m_header; }

private:
    explicit HeaderBar(Gtk::HeaderBar* h) : m_header(h) {}
    void pack_start_native(Gtk::Widget* w);
    void pack_end_native(Gtk::Widget* w);
    void set_title_widget_native(Gtk::Widget* w);
    Gtk::HeaderBar* m_header;
};

/**
 * SearchEntry - single-line text entry styled as a search field. The search-changed
 * event fires as the user types; stop_search fires when the user presses Escape.
 */
class SearchEntry {
public:
    static SearchEntry create();

    void set_placeholder_text(const std::string& text);
    void set_text(const std::string& text);
    std::string get_text() const;

    template <typename Callback>
    void on_search_changed(Callback&& callback) {
        connect_search_changed_impl(
            sigc::slot<void(const std::string&)>(
                [cb = std::forward<Callback>(callback)](const std::string& s) { cb(s); }));
    }

    template <typename Callback>
    void on_stop_search(Callback&& callback) {
        connect_stop_search_impl(
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_entry, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_entry, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_entry, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_entry, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_entry, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_entry, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_entry, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_entry, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_entry, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_entry, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_entry); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_entry, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_entry, t); }
    void grab_focus()             { detail::widget_grab_focus(m_entry); }
    Gtk::Widget* native_widget() const noexcept { return m_entry; }
    Gtk::SearchEntry* native() const noexcept { return m_entry; }

private:
    explicit SearchEntry(Gtk::SearchEntry* e) : m_entry(e) {}
    void connect_search_changed_impl(sigc::slot<void(const std::string&)> slot);
    void connect_stop_search_impl(sigc::slot<void()> slot);
    Gtk::SearchEntry* m_entry;
};

}  // namespace ase::adp::gtk
