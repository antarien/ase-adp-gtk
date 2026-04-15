/**
 * @file        gtk_widget.cpp
 * @brief       Implementation for widget.hpp wrappers + detail::widget_* helpers
 * @description Forwards every wrapper call to the underlying gtkmm method.
 *              Constructors use Gtk::make_managed<T>() for managed widgets
 *              (parented and lifecycle-tracked by gtkmm) so no raw new appears
 *              in this source.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/widget.hpp>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/searchentry.h>
#include <pangomm/layout.h>

namespace ase::adp::gtk {

namespace detail {

void widget_set_margin_start(Gtk::Widget* w, int m)  { w->set_margin_start(m); }
void widget_set_margin_end(Gtk::Widget* w, int m)    { w->set_margin_end(m); }
void widget_set_margin_top(Gtk::Widget* w, int m)    { w->set_margin_top(m); }
void widget_set_margin_bottom(Gtk::Widget* w, int m) { w->set_margin_bottom(m); }

void widget_set_margin_all(Gtk::Widget* w, int m) {
    w->set_margin_start(m);
    w->set_margin_end(m);
    w->set_margin_top(m);
    w->set_margin_bottom(m);
}

void widget_set_hexpand(Gtk::Widget* w, bool e) { w->set_hexpand(e); }
void widget_set_vexpand(Gtk::Widget* w, bool e) { w->set_vexpand(e); }

void widget_add_css_class(Gtk::Widget* w, const std::string& c)    { w->add_css_class(c); }
void widget_remove_css_class(Gtk::Widget* w, const std::string& c) { w->remove_css_class(c); }

void widget_set_visible(Gtk::Widget* w, bool v) { w->set_visible(v); }
bool widget_get_visible(const Gtk::Widget* w)   { return w->get_visible(); }

void widget_set_size_request(Gtk::Widget* w, int width, int height) {
    w->set_size_request(width, height);
}

void widget_set_tooltip_text(Gtk::Widget* w, const std::string& text) {
    w->set_tooltip_text(text);
}

void widget_grab_focus(Gtk::Widget* w) { w->grab_focus(); }

}  // namespace detail

// ── Box ─────────────────────────────────────────────────────────────

Box Box::horizontal(int spacing) {
    return Box(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, spacing));
}

Box Box::vertical(int spacing) {
    return Box(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, spacing));
}

Box Box::create(Orientation orientation, int spacing) {
    auto gtk_orient = (orientation == Orientation::Horizontal)
        ? Gtk::Orientation::HORIZONTAL
        : Gtk::Orientation::VERTICAL;
    return Box(Gtk::make_managed<Gtk::Box>(gtk_orient, spacing));
}

void Box::append_native(Gtk::Widget* w)  { m_box->append(*w); }
void Box::prepend_native(Gtk::Widget* w) { m_box->prepend(*w); }
void Box::remove_native(Gtk::Widget* w)  { m_box->remove(*w); }

void Box::remove_all_children() {
    while (auto* child = m_box->get_first_child()) {
        m_box->remove(*child);
    }
}

// ── Button ──────────────────────────────────────────────────────────

Button Button::create(const std::string& label) {
    auto* b = Gtk::make_managed<Gtk::Button>(label);
    return Button(b);
}

Button Button::create_from_icon(const std::string& icon_name) {
    auto* b = Gtk::make_managed<Gtk::Button>();
    b->set_icon_name(icon_name);
    return Button(b);
}

Button Button::create() {
    return Button(Gtk::make_managed<Gtk::Button>());
}

void Button::set_label(const std::string& label)         { m_button->set_label(label); }
void Button::set_icon_name(const std::string& icon_name) { m_button->set_icon_name(icon_name); }

void Button::connect_clicked_impl(sigc::slot<void()> slot) {
    m_button->signal_clicked().connect(std::move(slot));
}

// ── Label ───────────────────────────────────────────────────────────

Label Label::create(const std::string& text) {
    return Label(Gtk::make_managed<Gtk::Label>(text));
}

void Label::set_text(const std::string& text)     { m_label->set_text(text); }
void Label::set_markup(const std::string& markup) { m_label->set_markup(markup); }
std::string Label::get_text() const               { return m_label->get_text(); }

void Label::set_xalign(float align)               { m_label->set_xalign(align); }

void Label::enable_ellipsize_end() {
    m_label->set_ellipsize(Pango::EllipsizeMode::END);
}

// ── ScrolledWindow ──────────────────────────────────────────────────

ScrolledWindow ScrolledWindow::create() {
    auto* sw = Gtk::make_managed<Gtk::ScrolledWindow>();
    sw->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    return ScrolledWindow(sw);
}

void ScrolledWindow::set_child_native(Gtk::Widget* w) { m_sw->set_child(*w); }

// ── HeaderBar ───────────────────────────────────────────────────────

HeaderBar HeaderBar::create() {
    return HeaderBar(Gtk::make_managed<Gtk::HeaderBar>());
}

void HeaderBar::pack_start_native(Gtk::Widget* w) { m_header->pack_start(*w); }
void HeaderBar::pack_end_native(Gtk::Widget* w)   { m_header->pack_end(*w); }
void HeaderBar::set_title_widget_native(Gtk::Widget* w) { m_header->set_title_widget(*w); }

void HeaderBar::set_show_title_buttons(bool show) {
    m_header->set_show_title_buttons(show);
}

// ── SearchEntry ─────────────────────────────────────────────────────

SearchEntry SearchEntry::create() {
    return SearchEntry(Gtk::make_managed<Gtk::SearchEntry>());
}

void SearchEntry::set_placeholder_text(const std::string& text) {
    m_entry->set_placeholder_text(text);
}

void SearchEntry::set_text(const std::string& text) { m_entry->set_text(text); }
std::string SearchEntry::get_text() const           { return m_entry->get_text(); }

void SearchEntry::connect_search_changed_impl(sigc::slot<void(const std::string&)> slot) {
    auto* entry = m_entry;
    entry->signal_search_changed().connect([entry, slot]() {
        slot(entry->get_text());
    });
}

void SearchEntry::connect_stop_search_impl(sigc::slot<void()> slot) {
    m_entry->signal_stop_search().connect(std::move(slot));
}

}  // namespace ase::adp::gtk
