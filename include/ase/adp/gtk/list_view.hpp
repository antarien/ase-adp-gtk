#pragma once

/**
 * @file        list_view.hpp
 * @brief       ListView + ListItem + ListItemFactory + SingleSelection wrappers
 * @description Wrappers for the gtkmm-4 list rendering pipeline. The factory
 *              callback model uses templates instead of std::function. ListItem
 *              child access uses static_cast on a known type tag stored via
 *              g_object_set_data, sidestepping dynamic_cast entirely.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>
#include <ase/adp/gtk/widget.hpp>  // for detail::widget_* helpers

#include <gtkmm/listview.h>
#include <gtkmm/listitem.h>
#include <gtkmm/signallistitemfactory.h>
#include <gtkmm/singleselection.h>
#include <giomm/listmodel.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/**
 * ListItem - one row in a ListView, given to factory callbacks during the
 * setup/bind/unbind/teardown lifecycle. Wraps Gtk::ListItem, exposes the row's
 * model item via get_item_native() (returns the underlying GObject for the
 * caller to wrap with the appropriate model wrapper).
 */
class ListItem {
public:
    /** Set the widget shown for this list item (called once during setup). */
    template <typename W>
    void set_child(W& widget) { set_child_native(widget.native_widget()); }

    /** Returns the underlying Gtk::Widget* set as the row's child, or null. */
    Gtk::Widget* get_child_native() const noexcept;

    /** Returns the underlying GObject for this row's model item, or null. */
    Glib::RefPtr<Glib::ObjectBase> get_item_native() const noexcept;

    /** Returns the row position within the model, or guint(-1) if unbound. */
    unsigned int get_position() const noexcept;

    /** Internal: returns the underlying Gtk::ListItem refptr. */
    const Glib::RefPtr<Gtk::ListItem>& native() const noexcept { return m_item; }

    explicit ListItem(Glib::RefPtr<Gtk::ListItem> item) : m_item(std::move(item)) {}

private:
    void set_child_native(Gtk::Widget* w);
    Glib::RefPtr<Gtk::ListItem> m_item;
};

/**
 * ListItemFactory - signal-based factory that creates and binds row widgets for
 * a ListView. The four lifecycle callbacks (setup/bind/unbind/teardown) are
 * accepted as template callables; the factory wraps them in sigc::slot
 * internally. Most clients only need setup and bind.
 */
class ListItemFactory {
public:
    /** Create a new SignalListItemFactory wrapper. */
    static ListItemFactory create();

    /** Connect the setup callback (called once per widget creation). */
    template <typename Callback>
    void on_setup(Callback&& callback) {
        connect_setup_impl(
            sigc::slot<void(ListItem&)>(
                [cb = std::forward<Callback>(callback)](ListItem& item) { cb(item); }));
    }

    /** Connect the bind callback (called whenever a row is bound to a model item). */
    template <typename Callback>
    void on_bind(Callback&& callback) {
        connect_bind_impl(
            sigc::slot<void(ListItem&)>(
                [cb = std::forward<Callback>(callback)](ListItem& item) { cb(item); }));
    }

    /** Connect the unbind callback (called when a row is recycled). */
    template <typename Callback>
    void on_unbind(Callback&& callback) {
        connect_unbind_impl(
            sigc::slot<void(ListItem&)>(
                [cb = std::forward<Callback>(callback)](ListItem& item) { cb(item); }));
    }

    /** Connect the teardown callback (called once before widget destruction). */
    template <typename Callback>
    void on_teardown(Callback&& callback) {
        connect_teardown_impl(
            sigc::slot<void(ListItem&)>(
                [cb = std::forward<Callback>(callback)](ListItem& item) { cb(item); }));
    }

    /** Internal: returns the underlying Gtk::ListItemFactory refptr. */
    const Glib::RefPtr<Gtk::SignalListItemFactory>& native() const noexcept { return m_factory; }

private:
    explicit ListItemFactory(Glib::RefPtr<Gtk::SignalListItemFactory> f) : m_factory(std::move(f)) {}
    void connect_setup_impl(sigc::slot<void(ListItem&)> slot);
    void connect_bind_impl(sigc::slot<void(ListItem&)> slot);
    void connect_unbind_impl(sigc::slot<void(ListItem&)> slot);
    void connect_teardown_impl(sigc::slot<void(ListItem&)> slot);
    Glib::RefPtr<Gtk::SignalListItemFactory> m_factory;
};

/**
 * SingleSelection - selection model that allows at most one selected row at a
 * time. Wraps Gio::ListModel, used as the model for a ListView when paired with
 * a factory.
 */
class SingleSelection {
public:
    /** Create a SingleSelection wrapping the given ListModel-shaped wrapper. */
    template <typename Model>
    static SingleSelection create(Model& model) {
        return create_native(model.native_list_model());
    }

    void set_autoselect(bool autoselect);
    void set_can_unselect(bool can_unselect);

    /** Returns the position of the selected row, or guint(-1) if none. */
    unsigned int get_selected() const noexcept;
    void set_selected(unsigned int position);

    /** Returns the selected row item as a raw GObject (unwrap with the model's wrapper). */
    Glib::RefPtr<Glib::ObjectBase> get_selected_item_native() const noexcept;

    /** Internal: returns the underlying Gtk::SingleSelection refptr. */
    const Glib::RefPtr<Gtk::SingleSelection>& native() const noexcept { return m_selection; }

private:
    explicit SingleSelection(Glib::RefPtr<Gtk::SingleSelection> s) : m_selection(std::move(s)) {}
    static SingleSelection create_native(Glib::RefPtr<Gio::ListModel> model);
    Glib::RefPtr<Gtk::SingleSelection> m_selection;
};

/**
 * ListView - displays the rows of a SelectionModel using a ListItemFactory to
 * render each row. Standard hookup: model → SingleSelection → ListView, with
 * the factory bound separately.
 */
class ListView {
public:
    static ListView create();

    void set_model(SingleSelection& selection);
    void set_factory(ListItemFactory& factory);

    /** Connect a controller (gesture, key controller, drag source) to the list view. */
    template <typename Controller>
    void add_controller(Controller& controller) {
        add_controller_native(controller.native_controller());
    }

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_view, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_view, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_view, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_view, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_view, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_view, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_view, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_view, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_view, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_view, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_view); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_view, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_view, t); }
    void grab_focus()             { detail::widget_grab_focus(m_view); }
    Gtk::Widget* native_widget() const noexcept { return m_view; }
    Gtk::ListView* native() const noexcept { return m_view; }

private:
    explicit ListView(Gtk::ListView* v) : m_view(v) {}
    void add_controller_native(Glib::RefPtr<Gtk::EventController> controller);
    Gtk::ListView* m_view;
};

}  // namespace ase::adp::gtk
