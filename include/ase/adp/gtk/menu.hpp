#pragma once

/**
 * @file        menu.hpp
 * @brief       Menu + PopoverMenu + ActionGroup wrappers
 * @description Wrappers for the gtkmm-4 menu + action system. A Menu is the
 *              abstract menu model (entries with action names). An ActionGroup
 *              maps action names to callables. A PopoverMenu renders the menu
 *              model at a screen position.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <gtkmm/popovermenu.h>
#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/**
 * Menu - abstract menu model. Add labelled entries that reference action names
 * (e.g., "explorer.copy-path"). The entries appear in the order they are added.
 */
class Menu {
public:
    static Menu create();

    /** Append a labelled menu item bound to the given action name. */
    void append(const std::string& label, const std::string& action_name);

    /** Internal: returns the underlying Gio::Menu refptr. */
    const Glib::RefPtr<Gio::Menu>& native() const noexcept { return m_menu; }

private:
    explicit Menu(Glib::RefPtr<Gio::Menu> m) : m_menu(std::move(m)) {}
    Glib::RefPtr<Gio::Menu> m_menu;
};

/**
 * ActionGroup - maps action names to callable handlers. Install on a widget via
 * insert_action_group(prefix, group); menu items reference actions as
 * "prefix.action-name".
 */
class ActionGroup {
public:
    static ActionGroup create();

    /** Add a parameterless action with a callable handler. */
    template <typename Callback>
    void add_action(const std::string& name, Callback&& callback) {
        add_action_impl(name,
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    /** Internal: returns the underlying Gio::SimpleActionGroup refptr. */
    const Glib::RefPtr<Gio::SimpleActionGroup>& native() const noexcept { return m_group; }

private:
    explicit ActionGroup(Glib::RefPtr<Gio::SimpleActionGroup> g) : m_group(std::move(g)) {}
    void add_action_impl(const std::string& name, sigc::slot<void()> handler);
    Glib::RefPtr<Gio::SimpleActionGroup> m_group;
};

/**
 * PopoverMenu - floating menu rendered at a screen position. Built from a Menu
 * model and parented to a widget. Call popup() to show it; the user closes it
 * by selecting an item or clicking elsewhere.
 */
class PopoverMenu {
public:
    /** Create a PopoverMenu from a Menu model. */
    static PopoverMenu create_from_menu(Menu& menu);

    /** Install this popover on a widget (parent owns lifecycle). */
    template <typename W>
    void set_parent(W& parent) { set_parent_native(parent.native_widget()); }

    /** Position the popover at (x, y) coordinates relative to its parent. */
    void set_pointing_to(int x, int y, int width = 1, int height = 1);

    /** Show the popover. */
    void popup();

private:
    explicit PopoverMenu(Gtk::PopoverMenu* p) : m_popover(p) {}
    void set_parent_native(Gtk::Widget* parent);
    Gtk::PopoverMenu* m_popover;
};

/**
 * Free helper - install an action group on any widget under the given prefix.
 * Used together with Menu and PopoverMenu.
 */
template <typename W>
void insert_action_group(W& widget, const std::string& prefix, ActionGroup& group);

namespace detail {
void widget_insert_action_group(
    Gtk::Widget* w, const std::string& prefix, Glib::RefPtr<Gio::SimpleActionGroup> group);
}

template <typename W>
void insert_action_group(W& widget, const std::string& prefix, ActionGroup& group) {
    detail::widget_insert_action_group(widget.native_widget(), prefix, group.native());
}

}  // namespace ase::adp::gtk
