#pragma once

/**
 * @file        io.hpp
 * @brief       File + FileMonitor + FileLauncher + Clipboard wrappers
 * @description Wrappers for gtkmm-4 file system I/O. File represents a path,
 *              FileMonitor watches for changes, FileLauncher opens a file with
 *              the desktop default application, Clipboard handles text copy.
 *              FileInfo lives in tree.hpp because it is tightly coupled with
 *              DirectoryList and TreeListModel there.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <giomm/file.h>
#include <giomm/filemonitor.h>
#include <gtkmm/filelauncher.h>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>
#include <glibmm/refptr.h>
#include <sigc++/connection.h>

#include <memory>
#include <string>
#include <utility>

namespace ase::adp::gtk {

/**
 * File - lightweight path handle. Wraps Gio::File. Used as the input to
 * FileLauncher and FileMonitor. Construction does not perform any I/O.
 */
class File {
public:
    /** Create a File handle for an absolute filesystem path. */
    static File create_for_path(const std::string& path);

    /** Returns the absolute path string. */
    std::string get_path() const;

    /** Internal: returns the underlying Gio::File refptr. */
    const Glib::RefPtr<Gio::File>& native() const noexcept { return m_file; }

private:
    explicit File(Glib::RefPtr<Gio::File> f) : m_file(std::move(f)) {}
    Glib::RefPtr<Gio::File> m_file;
};

/**
 * FileMonitor - watches a directory or file for changes. Clients connect a
 * single on_changed handler that fires on any modification (create, delete,
 * rename, content change). The handler is invoked on the GLib main thread.
 *
 * Optional debouncing coalesces rapid bursts (batch rename, VCS checkout) into
 * a single callback fired `debounce_ms` after the last event. Pass 0 to fire
 * synchronously per change.
 */
class FileMonitor {
public:
    /** Begin monitoring the directory at the given File with optional debounce. */
    static FileMonitor monitor_directory(File& file, int debounce_ms = 0);

    /** Connect a callable invoked on any change: void(). */
    template <typename Callback>
    void on_changed(Callback&& callback) {
        connect_changed_impl(
            sigc::slot<void()>([cb = std::forward<Callback>(callback)]() { cb(); }));
    }

    /** Internal: returns the underlying Gio::FileMonitor refptr. */
    const Glib::RefPtr<Gio::FileMonitor>& native() const noexcept { return m_state->monitor; }

private:
    // Shared state lets copies of FileMonitor observe the same debounce timer.
    struct State {
        Glib::RefPtr<Gio::FileMonitor> monitor;
        int debounce_ms = 0;
        sigc::connection timer;
        sigc::slot<void()> user_slot;
    };

    explicit FileMonitor(std::shared_ptr<State> state) : m_state(std::move(state)) {}
    void connect_changed_impl(sigc::slot<void()> slot);
    std::shared_ptr<State> m_state;
};

/**
 * FileLauncher - opens a file with the desktop default application. The launch
 * is asynchronous; the optional completion callback fires once the launch
 * succeeds or fails. Used for the explorer's double-click-to-open behaviour.
 */
class FileLauncher {
public:
    /** Create a launcher for the given File. */
    static FileLauncher create(File& file);

    /** Launch the file. The window argument is the parent for any error dialogs. */
    template <typename ParentWindow>
    void launch(ParentWindow& window) {
        launch_native(window.native_widget());
    }

    /** Open the containing folder of this file in the file manager. */
    template <typename ParentWindow>
    void open_containing_folder(ParentWindow& window) {
        open_containing_folder_native(window.native_widget());
    }

private:
    explicit FileLauncher(Glib::RefPtr<Gtk::FileLauncher> l) : m_launcher(std::move(l)) {}
    void launch_native(Gtk::Widget* parent);
    void open_containing_folder_native(Gtk::Widget* parent);
    Glib::RefPtr<Gtk::FileLauncher> m_launcher;
};

/**
 * Clipboard - free helper to copy text to the system clipboard. Operates on a
 * widget (clipboard is per-display, looked up via the widget's display).
 */
namespace detail {
void widget_copy_to_clipboard(Gtk::Widget* w, const std::string& text);
}

template <typename W>
void copy_to_clipboard(W& widget, const std::string& text) {
    detail::widget_copy_to_clipboard(widget.native_widget(), text);
}

/**
 * spawn_command_async - launches a shell command line asynchronously via the
 * GLib process spawner. Returns immediately; the command runs detached from
 * the application. Used for actions like "Open in Terminal" that shell out to
 * external processes. Failures are logged via Glib::spawn_error.
 */
void spawn_command_async(const std::string& command_line);

}  // namespace ase::adp::gtk
