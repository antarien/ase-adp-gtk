/**
 * @file        gtk_menu.cpp
 * @brief       Implementation for menu.hpp wrappers
 * @description Menu uses Gio::Menu::create. ActionGroup uses Gio::SimpleAction
 *              with the supplied slot. PopoverMenu uses Gtk::make_managed for
 *              the popover and forwards parent/positioning calls.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/menu.hpp>

#include <gtkmm/popovermenu.h>
#include <gdkmm/rectangle.h>
#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

namespace ase::adp::gtk {

// ── Menu ────────────────────────────────────────────────────────────

Menu Menu::create() {
    return Menu(Gio::Menu::create());
}

void Menu::append(const std::string& label, const std::string& action_name) {
    m_menu->append(label, action_name);
}

// ── ActionGroup ─────────────────────────────────────────────────────

ActionGroup ActionGroup::create() {
    return ActionGroup(Gio::SimpleActionGroup::create());
}

void ActionGroup::add_action_impl(const std::string& name, sigc::slot<void()> handler) {
    m_group->add_action(name, std::move(handler));
}

// ── PopoverMenu ─────────────────────────────────────────────────────

PopoverMenu PopoverMenu::create_from_menu(Menu& menu) {
    auto* p = Gtk::make_managed<Gtk::PopoverMenu>(menu.native());
    return PopoverMenu(p);
}

void PopoverMenu::set_parent_native(Gtk::Widget* parent) {
    m_popover->set_parent(*parent);
}

void PopoverMenu::set_pointing_to(int x, int y, int width, int height) {
    m_popover->set_pointing_to(Gdk::Rectangle(x, y, width, height));
}

void PopoverMenu::popup() {
    m_popover->popup();
}

// ── insert_action_group helper ──────────────────────────────────────

namespace detail {

void widget_insert_action_group(
    Gtk::Widget* w, const std::string& prefix, Glib::RefPtr<Gio::SimpleActionGroup> group)
{
    w->insert_action_group(prefix, group);
}

}  // namespace detail

}  // namespace ase::adp::gtk
