#pragma once

/**
 * @file        style.hpp
 * @brief       CssProvider wrapper
 * @description Loads CSS strings or files into the gtkmm style system. Once
 *              installed via install_for_default_display(), the rules apply to
 *              every widget on that display at the application priority level.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <gtkmm/cssprovider.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/**
 * CssProvider - holds a parsed CSS rule set and installs it on the default
 * display. Use load_from_data() to feed in a generated stylesheet, then
 * install_for_default_display() once at startup.
 */
class CssProvider {
public:
    static CssProvider create();

    /** Parse the given CSS source string. */
    void load_from_data(const std::string& css);

    /** Install this provider on the default display at application priority. */
    void install_for_default_display();

    /** Internal: returns the underlying Gtk::CssProvider refptr. */
    const Glib::RefPtr<Gtk::CssProvider>& native() const noexcept { return m_provider; }

private:
    explicit CssProvider(Glib::RefPtr<Gtk::CssProvider> p) : m_provider(std::move(p)) {}
    Glib::RefPtr<Gtk::CssProvider> m_provider;
};

}  // namespace ase::adp::gtk
