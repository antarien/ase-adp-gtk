/**
 * @file        gtk_list_view.cpp
 * @brief       Implementation for list_view.hpp wrappers
 * @description Forwards every wrapper call to the underlying gtkmm types and
 *              wraps each ListItemFactory signal in a sigc::slot constructor
 *              that re-wraps the gtkmm Glib::RefPtr<Gtk::ListItem> as an
 *              ase::adp::gtk::ListItem before invoking the user callable.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/list_view.hpp>

#include <gtkmm/listview.h>
#include <gtkmm/listitem.h>
#include <gtkmm/signallistitemfactory.h>
#include <gtkmm/singleselection.h>
#include <giomm/listmodel.h>

namespace ase::adp::gtk {

// ── ListItem ────────────────────────────────────────────────────────

void ListItem::set_child_native(Gtk::Widget* w) {
    m_item->set_child(*w);
}

Gtk::Widget* ListItem::get_child_native() const noexcept {
    return m_item->get_child();
}

Glib::RefPtr<Glib::ObjectBase> ListItem::get_item_native() const noexcept {
    return m_item->get_item();
}

unsigned int ListItem::get_position() const noexcept {
    return m_item->get_position();
}

// ── ListItemFactory ─────────────────────────────────────────────────

ListItemFactory ListItemFactory::create() {
    return ListItemFactory(Gtk::SignalListItemFactory::create());
}

void ListItemFactory::connect_setup_impl(sigc::slot<void(ListItem&)> slot) {
    m_factory->signal_setup().connect([slot](const Glib::RefPtr<Gtk::ListItem>& item) {
        ListItem wrapped(item);
        slot(wrapped);
    });
}

void ListItemFactory::connect_bind_impl(sigc::slot<void(ListItem&)> slot) {
    m_factory->signal_bind().connect([slot](const Glib::RefPtr<Gtk::ListItem>& item) {
        ListItem wrapped(item);
        slot(wrapped);
    });
}

void ListItemFactory::connect_unbind_impl(sigc::slot<void(ListItem&)> slot) {
    m_factory->signal_unbind().connect([slot](const Glib::RefPtr<Gtk::ListItem>& item) {
        ListItem wrapped(item);
        slot(wrapped);
    });
}

void ListItemFactory::connect_teardown_impl(sigc::slot<void(ListItem&)> slot) {
    m_factory->signal_teardown().connect([slot](const Glib::RefPtr<Gtk::ListItem>& item) {
        ListItem wrapped(item);
        slot(wrapped);
    });
}

// ── SingleSelection ─────────────────────────────────────────────────

SingleSelection SingleSelection::create_native(Glib::RefPtr<Gio::ListModel> model) {
    return SingleSelection(Gtk::SingleSelection::create(model));
}

void SingleSelection::set_autoselect(bool autoselect)     { m_selection->set_autoselect(autoselect); }
void SingleSelection::set_can_unselect(bool can_unselect) { m_selection->set_can_unselect(can_unselect); }

unsigned int SingleSelection::get_selected() const noexcept {
    return m_selection->get_selected();
}

void SingleSelection::set_selected(unsigned int position) {
    m_selection->set_selected(position);
}

Glib::RefPtr<Glib::ObjectBase> SingleSelection::get_selected_item_native() const noexcept {
    return m_selection->get_selected_item();
}

// ── ListView ────────────────────────────────────────────────────────

ListView ListView::create() {
    return ListView(Gtk::make_managed<Gtk::ListView>());
}

void ListView::set_model(SingleSelection& selection) {
    m_view->set_model(selection.native());
}

void ListView::set_factory(ListItemFactory& factory) {
    m_view->set_factory(factory.native());
}

void ListView::add_controller_native(Glib::RefPtr<Gtk::EventController> controller) {
    m_view->add_controller(controller);
}

}  // namespace ase::adp::gtk
