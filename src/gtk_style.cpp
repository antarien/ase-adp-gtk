/**
 * @file        gtk_style.cpp
 * @brief       Implementation for style.hpp wrappers
 * @description CssProvider parses CSS source and installs it on the default
 *              display at GTK_STYLE_PROVIDER_PRIORITY_APPLICATION priority.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/style.hpp>

#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gdkmm/display.h>

namespace ase::adp::gtk {

CssProvider CssProvider::create() {
    return CssProvider(Gtk::CssProvider::create());
}

void CssProvider::load_from_data(const std::string& css) {
    m_provider->load_from_data(css);
}

void CssProvider::install_for_default_display() {
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        m_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

}  // namespace ase::adp::gtk
