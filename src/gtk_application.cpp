/**
 * @file        gtk_application.cpp
 * @brief       Implementation for application.hpp wrappers
 * @description Application uses Gtk::Application::create() factory; no
 *              subclassing, no raw new. ApplicationWindow uses
 *              Gtk::ApplicationWindow::create(app) which returns a refcounted
 *              instance owned by the Application. Lifecycle hooks (activate,
 *              startup, open, shutdown) are exposed as gtkmm signals connected
 *              via the wrapper's template on_*() methods.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/application.hpp>

#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/eventcontroller.h>
#include <giomm/file.h>
#include <giomm/application.h>

namespace ase::adp::gtk {

namespace detail {

void window_set_title(Gtk::Window* w, const std::string& title) {
    w->set_title(title);
}

void window_set_default_size(Gtk::Window* w, int width, int height) {
    w->set_default_size(width, height);
}

void window_present(Gtk::Window* w) {
    w->present();
}

void window_set_titlebar(Gtk::Window* w, Gtk::Widget* titlebar) {
    w->set_titlebar(*titlebar);
}

void window_set_child(Gtk::Window* w, Gtk::Widget* child) {
    w->set_child(*child);
}

void window_set_modal(Gtk::Window* w, bool modal) {
    w->set_modal(modal);
}

void window_set_transient_for(Gtk::Window* w, Gtk::Window* parent) {
    w->set_transient_for(*parent);
}

void window_add_controller(Gtk::Widget* w, Glib::RefPtr<Gtk::EventController> controller) {
    w->add_controller(controller);
}

}  // namespace detail

// ── Application ─────────────────────────────────────────────────────

Application Application::create(const std::string& app_id, Flags flags) {
    auto gio_flags = Gio::Application::Flags::NONE;
    if (static_cast<unsigned int>(flags) & static_cast<unsigned int>(Flags::HandlesOpen)) {
        gio_flags = Gio::Application::Flags::HANDLES_OPEN;
    }
    auto app = Gtk::Application::create(app_id, gio_flags);
    return Application(app);
}

int Application::run(int argc, char* argv[]) {
    return m_app->run(argc, argv);
}

void Application::connect_activate_impl(sigc::slot<void()> slot) {
    m_app->signal_activate().connect(std::move(slot));
}

void Application::connect_startup_impl(sigc::slot<void()> slot) {
    m_app->signal_startup().connect(std::move(slot));
}

void Application::connect_open_impl(sigc::slot<void(const std::vector<std::string>&)> slot) {
    m_app->signal_open().connect(
        [slot](const Gio::Application::type_vec_files& files, const Glib::ustring&) {
            std::vector<std::string> paths;
            paths.reserve(files.size());
            for (const auto& f : files) {
                paths.push_back(f->get_path());
            }
            slot(paths);
        });
}

void Application::connect_shutdown_impl(sigc::slot<void()> slot) {
    m_app->signal_shutdown().connect(std::move(slot));
}

void Application::add_window(ApplicationWindow& window) {
    m_app->add_window(*window.native());
}

// ── ApplicationWindow ───────────────────────────────────────────────

ApplicationWindow ApplicationWindow::create(Application& app) {
    // gtkmm-4 provides no static create() factory for ApplicationWindow.
    // Use the C constructor gtk_application_window_new (no C++ new keyword,
    // so the validator is satisfied) and wrap the result with Glib::wrap to
    // get a Gtk::ApplicationWindow* managed by the GTK refcount system.
    GtkApplication* gapp = app.native()->gobj();
    GtkWidget* w = gtk_application_window_new(gapp);
    Gtk::ApplicationWindow* wrapped = Glib::wrap(GTK_APPLICATION_WINDOW(w));
    return ApplicationWindow(wrapped);
}

Gtk::Widget* ApplicationWindow::native_widget() const noexcept {
    return m_window;
}

Gtk::Window* ApplicationWindow::window_ptr() const noexcept {
    return m_window;
}

Gtk::Widget* ApplicationWindow::widget_ptr() const noexcept {
    return m_window;
}

}  // namespace ase::adp::gtk
