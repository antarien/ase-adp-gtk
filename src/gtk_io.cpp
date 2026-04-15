/**
 * @file        gtk_io.cpp
 * @brief       Implementation for io.hpp wrappers
 * @description File wraps Gio::File path handles. FileMonitor wires the
 *              signal_changed firing to a single user callable. FileLauncher
 *              forwards launch and open_containing_folder with a discarded
 *              completion callback. Clipboard helper looks up the per-display
 *              clipboard via the parent widget.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/io.hpp>

#include <giomm/file.h>
#include <giomm/filemonitor.h>
#include <gtkmm/filelauncher.h>
#include <gtkmm/widget.h>
#include <gdkmm/clipboard.h>
#include <glibmm/main.h>
#include <glibmm/spawn.h>
#include <glibmm/error.h>

namespace ase::adp::gtk {

namespace {
// Walk up the widget tree to the root window via the C API. gtk_widget_get_root
// returns a GtkRoot* which is an interface; for top-level widgets the underlying
// object is always a GtkWindow, confirmed by GTK_IS_WINDOW before wrapping.
Gtk::Window* root_window(Gtk::Widget* w) {
    if (!w) return nullptr;
    GtkRoot* groot = gtk_widget_get_root(w->gobj());
    if (!groot || !GTK_IS_WINDOW(groot)) return nullptr;
    return Glib::wrap(GTK_WINDOW(groot));
}
}  // namespace

// ── File ────────────────────────────────────────────────────────────

File File::create_for_path(const std::string& path) {
    return File(Gio::File::create_for_path(path));
}

std::string File::get_path() const {
    return m_file ? m_file->get_path() : std::string{};
}

// ── FileMonitor ─────────────────────────────────────────────────────

FileMonitor FileMonitor::monitor_directory(File& file, int debounce_ms) {
    auto state = std::make_shared<State>();
    state->monitor = file.native()->monitor_directory(Gio::FileMonitorFlags::WATCH_MOVES);
    state->debounce_ms = debounce_ms;
    return FileMonitor(std::move(state));
}

void FileMonitor::connect_changed_impl(sigc::slot<void()> slot) {
    m_state->user_slot = std::move(slot);
    auto state = m_state;  // capture shared_ptr so the slot survives even if the
                           // caller drops its copy of the FileMonitor wrapper.

    state->monitor->signal_changed().connect(
        [state](const Glib::RefPtr<Gio::File>&,
                const Glib::RefPtr<Gio::File>&,
                Gio::FileMonitor::Event) {
            if (state->debounce_ms <= 0) {
                if (state->user_slot) state->user_slot();
                return;
            }
            // Coalesce rapid bursts: cancel any pending timer, reschedule.
            if (state->timer.connected()) state->timer.disconnect();
            state->timer = Glib::signal_timeout().connect(
                [state]() -> bool {
                    if (state->user_slot) state->user_slot();
                    return false;  // one-shot
                },
                state->debounce_ms);
        });
}

// ── FileLauncher ────────────────────────────────────────────────────

FileLauncher FileLauncher::create(File& file) {
    return FileLauncher(Gtk::FileLauncher::create(file.native()));
}

void FileLauncher::launch_native(Gtk::Widget* parent) {
    auto* window = root_window(parent);
    if (window) {
        m_launcher->launch(*window, [](Glib::RefPtr<Gio::AsyncResult>&) {});
    }
}

void FileLauncher::open_containing_folder_native(Gtk::Widget* parent) {
    auto* window = root_window(parent);
    if (window) {
        m_launcher->open_containing_folder(*window, [](Glib::RefPtr<Gio::AsyncResult>&) {});
    }
}

// ── Clipboard helper ────────────────────────────────────────────────

namespace detail {

void widget_copy_to_clipboard(Gtk::Widget* w, const std::string& text) {
    auto clipboard = w->get_clipboard();
    if (clipboard) {
        clipboard->set_text(text);
    }
}

}  // namespace detail

// ── Command spawn helper ────────────────────────────────────────────

void spawn_command_async(const std::string& command_line) {
    try {
        Glib::spawn_command_line_async(command_line);
    } catch (const Glib::Error&) {
        // Detached spawn failures are non-fatal - swallow to keep the caller
        // decoupled from Glib error types. Hook up logging here when the
        // adapter gains an ase::log dependency.
    }
}

}  // namespace ase::adp::gtk
