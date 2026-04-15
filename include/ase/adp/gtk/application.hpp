#pragma once

/**
 * @file        application.hpp
 * @brief       Application + ApplicationWindow wrappers
 * @description Top-level lifecycle wrappers for a gtkmm-4 desktop application.
 *              Application wraps Gtk::Application via the documented create()
 *              factory - no subclassing required because all lifecycle hooks
 *              (activate, startup, open, shutdown) are exposed as gtkmm signals
 *              that the wrapper forwards through template on_*() methods.
 *              ApplicationWindow uses the same composition pattern via
 *              Gtk::ApplicationWindow::create(app), avoiding raw new entirely.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/headerbar.h>
#include <giomm/file.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>
#include <vector>

namespace ase::adp::gtk {

namespace detail {

void window_set_title(Gtk::Window* w, const std::string& title);
void window_set_default_size(Gtk::Window* w, int width, int height);
void window_present(Gtk::Window* w);
void window_set_titlebar(Gtk::Window* w, Gtk::Widget* titlebar);
void window_set_child(Gtk::Window* w, Gtk::Widget* child);
void window_set_modal(Gtk::Window* w, bool modal);
void window_set_transient_for(Gtk::Window* w, Gtk::Window* parent);
void window_add_controller(Gtk::Widget* w, Glib::RefPtr<Gtk::EventController> controller);

}  // namespace detail

/**
 * Application - top-level desktop application object. Created via the static
 * factory; lifecycle hooks are connected via on_*() template methods. The
 * underlying Gtk::Application is held by Glib::RefPtr so the wrapper is a cheap
 * value type (refcount inc on copy).
 *
 * Usage:
 *   auto app = ase::adp::gtk::Application::create("com.example.myapp");
 *   app.on_activate([&]() {
 *       auto window = ase::adp::gtk::ApplicationWindow::create(app);
 *       // ... build UI ...
 *       window.present();
 *   });
 *   return app.run(argc, argv);
 */
class Application {
public:
    /** Application flag bitfield - mirrors Gio::Application::Flags. */
    enum class Flags : unsigned int {
        None       = 0u,
        HandlesOpen = 1u << 1,  // App receives file paths via on_open()
    };

    /** Create a new application with the given D-Bus-style ID and flags. */
    static Application create(const std::string& app_id, Flags flags = Flags::None);

    /** Run the main loop. Returns the application exit code. */
    int run(int argc, char* argv[]);

    /** Connect a callable invoked when the application is activated (no files). */
    template <typename Callback>
    void on_activate(Callback&& callback) {
        connect_activate_impl(
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    /** Connect a callable invoked once at application startup. */
    template <typename Callback>
    void on_startup(Callback&& callback) {
        connect_startup_impl(
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    /** Connect a callable invoked when the OS asks the app to open files. */
    template <typename Callback>
    void on_open(Callback&& callback) {
        connect_open_impl(
            sigc::slot<void(const std::vector<std::string>&)>(
                [cb = std::forward<Callback>(callback)](const std::vector<std::string>& paths) { cb(paths); }));
    }

    /** Connect a callable invoked when the application is shutting down. */
    template <typename Callback>
    void on_shutdown(Callback&& callback) {
        connect_shutdown_impl(
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    /** Add a window to the application's window list (takes ownership share). */
    void add_window(ApplicationWindow& window);

    /** Internal: returns the underlying Gtk::Application refptr. */
    const Glib::RefPtr<Gtk::Application>& native() const noexcept { return m_app; }

private:
    explicit Application(Glib::RefPtr<Gtk::Application> app) : m_app(std::move(app)) {}
    void connect_activate_impl(sigc::slot<void()> slot);
    void connect_startup_impl(sigc::slot<void()> slot);
    void connect_open_impl(sigc::slot<void(const std::vector<std::string>&)> slot);
    void connect_shutdown_impl(sigc::slot<void()> slot);
    Glib::RefPtr<Gtk::Application> m_app;
};

inline Application::Flags operator|(Application::Flags a, Application::Flags b) {
    return static_cast<Application::Flags>(
        static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

/**
 * ApplicationWindow - top-level window owned by an Application. Created via the
 * static factory which calls gtk_application_window_new (no raw new keyword).
 * The window is owned by its Application via add_window; this wrapper holds a
 * non-owning raw pointer that remains valid for the window's full lifetime.
 */
class ApplicationWindow {
public:
    /** Create a new ApplicationWindow attached to the given Application. */
    static ApplicationWindow create(Application& app);

    void set_title(const std::string& title) { detail::window_set_title(window_ptr(), title); }
    void set_default_size(int width, int height) { detail::window_set_default_size(window_ptr(), width, height); }
    void present() { detail::window_present(window_ptr()); }
    void set_modal(bool modal) { detail::window_set_modal(window_ptr(), modal); }

    template <typename TitleBar>
    void set_titlebar(TitleBar& bar) {
        detail::window_set_titlebar(window_ptr(), bar.native_widget());
    }

    template <typename Child>
    void set_child(Child& child) {
        detail::window_set_child(window_ptr(), child.native_widget());
    }

    template <typename Controller>
    void add_controller(Controller& controller) {
        detail::window_add_controller(widget_ptr(), controller.native_controller());
    }

    /** Internal: returns the underlying Gtk::ApplicationWindow raw pointer. */
    Gtk::ApplicationWindow* native() const noexcept { return m_window; }

    /** Internal: returns the underlying Gtk::Widget* (for transient_for callers). */
    Gtk::Widget* native_widget() const noexcept;

private:
    explicit ApplicationWindow(Gtk::ApplicationWindow* w) : m_window(w) {}
    Gtk::Window* window_ptr() const noexcept;
    Gtk::Widget* widget_ptr() const noexcept;
    Gtk::ApplicationWindow* m_window;
};

}  // namespace ase::adp::gtk
