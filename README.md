# ase-adp-gtk

[![Layer](https://img.shields.io/badge/Layer-Adapter-orange.svg)]()
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)]()
[![Kind](https://img.shields.io/badge/Kind-3rd%20Party%20Isolation-red.svg)]()

> gtkmm-4 + libadwaita adapter — isolates `Gtk::*` / `Gio::*` / `Glib::*` / `Adw*` from client code

Part of [ASE - Antares Simulation Engine](../../..)

## Why this module exists

ASE desktop clients (the project explorer, the documentation viewer, the curator UI) use GTK4 with libadwaita as their native rendering toolkit. gtkmm-4's API is fundamentally inheritance- and pointer-cast-based: an application is a class deriving from `Gtk::Application`, a window derives from `Gtk::ApplicationWindow`, list factories require `std::dynamic_pointer_cast<Gtk::TreeListRow>`, and custom data items derive from `Glib::Object`. Every one of those patterns is forbidden by the ECS validator outside of explicitly exempted paths. Without an adapter, every client would either bypass the validator (forbidden by Rule 1) or rewrite its UI in something other than GTK (defeats the point of having a native client at all).

`ase-adp-gtk` is the architectural answer. It is the one and only place in the codebase where `class FooWindow : public Gtk::ApplicationWindow` is permitted, where `Gtk::make_managed<T>()` and `Glib::RefPtr<T>::cast_dynamic<>()` live, and where the boilerplate of registering custom GObject types via `g_object_new()` is hidden. Clients import `ase/adp/gtk/*.hpp` headers and never touch `Gtk::`, `Glib::`, `Gio::`, or `Adw*` directly. The result is that every client subgit stays gtkmm-clean: no inheritance from third-party bases, no raw `new`, no `dynamic_cast`, no `std::filesystem`, just `ase::adp::gtk::Window`, `ase::adp::gtk::Box`, `ase::adp::gtk::ListView`, `ase::adp::adw::PreferencesPage`, and so on.

The pattern is the same as `ase-adp-microtex`: one isolation subgit per inheritance-heavy third-party library, listed in `.claude/hooks/ecs_validator/data/third_party_oop.json` with explicit `allowed_bases`. This adapter declares `["Gtk::", "Glib::", "Gio::", "sigc::", "Cairo::", "Gdk::", "Pango::"]`. All other rules (naming, Doxygen headers, `std::`-forbid via `file_filter`, ECS anti-patterns on the wrapper's own data classes) remain fully active inside the adapter.

## Public API

Clients include the umbrella header and use only the `ase::adp::gtk` and `ase::adp::adw` namespaces:

```cpp
#include <ase/adp/gtk/gtk.hpp>
#include <ase/adp/adw/adw.hpp>

int main(int argc, char* argv[]) {
    auto app = ase::adp::gtk::Application::create("com.antarien.ase.explorer");
    app->on_activate([&app]() {
        auto window = ase::adp::gtk::ApplicationWindow::create(*app);
        window->set_title("ASE Explorer");
        window->set_default_size(380, 900);

        auto box = ase::adp::gtk::Box::create_vertical();
        auto button = ase::adp::gtk::Button::create("Hello");
        button->on_clicked([]() { /* ... */ });
        box->append(*button);

        window->set_child(*box);
        window->present();
    });
    return app->run(argc, argv);
}
```

## Wrapped surface

| Category      | gtkmm/Adw class                       | ase::adp::gtk wrapper                  |
|---------------|---------------------------------------|-----------------------------------|
| Application   | `Gtk::Application`                    | `ase::adp::gtk::Application`           |
| Window        | `Gtk::ApplicationWindow`              | `ase::adp::gtk::ApplicationWindow`     |
| Window        | `Gtk::Window`                         | `ase::adp::gtk::Window`                |
| Container     | `Gtk::Box`                            | `ase::adp::gtk::Box`                   |
| Container     | `Gtk::ScrolledWindow`                 | `ase::adp::gtk::ScrolledWindow`        |
| Control       | `Gtk::Button`                         | `ase::adp::gtk::Button`                |
| Control       | `Gtk::Label`                          | `ase::adp::gtk::Label`                 |
| Control       | `Gtk::SearchEntry`                    | `ase::adp::gtk::SearchEntry`           |
| Header        | `Gtk::HeaderBar`                      | `ase::adp::gtk::HeaderBar`             |
| List/Tree     | `Gtk::ListView`                       | `ase::adp::gtk::ListView`              |
| List/Tree     | `Gtk::TreeListModel` / `TreeListRow`  | `ase::adp::gtk::TreeListModel` / `Row` |
| List/Tree     | `Gtk::DirectoryList`                  | `ase::adp::gtk::DirectoryList`         |
| List/Tree     | `Gtk::SingleSelection`                | `ase::adp::gtk::SingleSelection`       |
| List/Tree     | `Gtk::FilterListModel`                | `ase::adp::gtk::FilterListModel`       |
| Factory       | `Gtk::SignalListItemFactory`          | `ase::adp::gtk::ListItemFactory`       |
| Menu/Popup    | `Gtk::PopoverMenu` / `Gio::Menu`      | `ase::adp::gtk::PopoverMenu` / `Menu`  |
| Gesture       | `Gtk::GestureClick`                   | `ase::adp::gtk::ClickGesture`          |
| Gesture       | `Gtk::DragSource` / `ContentProvider` | `ase::adp::gtk::DragSource`            |
| Gesture       | `Gtk::EventControllerKey`             | `ase::adp::gtk::KeyController`         |
| File I/O      | `Gio::File` / `Gtk::FileLauncher`     | `ase::adp::gtk::File` / `FileLauncher` |
| File I/O      | `Gio::FileMonitor`                    | `ase::adp::gtk::FileMonitor`           |
| File I/O      | `Gio::FileInfo`                       | `ase::adp::gtk::FileInfo`              |
| Style         | `Gtk::CssProvider` / `StyleContext`   | `ase::adp::gtk::CssProvider`           |
| Adwaita       | `AdwApplicationWindow`                | `ase::adp::adw::ApplicationWindow`     |
| Adwaita       | `AdwWindow`                           | `ase::adp::adw::Window`                |
| Adwaita       | `AdwToolbarView`                      | `ase::adp::adw::ToolbarView`           |
| Adwaita       | `AdwHeaderBar`                        | `ase::adp::adw::HeaderBar`             |
| Adwaita       | `AdwViewStack` / `AdwViewSwitcher`    | `ase::adp::adw::ViewStack` / `Switcher`|
| Adwaita       | `AdwPreferencesPage` / `Group` / `Row`| `ase::adp::adw::PreferencesPage` / ... |
| Adwaita       | `AdwSwitchRow` / `AdwEntryRow`        | `ase::adp::adw::SwitchRow` / `EntryRow`|

## Architecture

- **Inheritance:** Wrappers internally subclass gtkmm classes only when required by the gtkmm plugin model (Application, ApplicationWindow). Most wrappers compose by holding a `Glib::RefPtr<Gtk::Foo>` member.
- **No raw `new`:** Wrapper factories use `Gtk::make_managed<T>()`, `T::create()` static factories, or `g_object_new()` (no `new` keyword in source). The `RAW_NEW_DELETE_FORBIDDEN` rule is fully active inside this adapter — same as microtex-adapter.
- **No `dynamic_cast`:** Wrappers use `Glib::RefPtr<T>::cast_dynamic<U>()` and `static_cast<>` for known-type widget tree access. Same as microtex-adapter.
- **Signal forwarding:** Wrappers expose `on_*(callable)` methods that internally connect to gtkmm signals, so client code never imports `<sigc++/...>` and never uses `signal_xxx().connect(...)`.
- **Custom GObject types:** Where clients used to derive from `Glib::Object` (e.g. for `Gio::ListStore<MyType>`), the adapter provides type-erased `ase::adp::gtk::DataObject` registered via `G_DEFINE_TYPE` and instantiated via `g_object_new()` — no client-side subclassing required.

## Layer

`adapter/` is a third-party isolation layer orthogonal to the `L0..L5` ECS stack. Consumed by L5 desktop clients (`ase-client-explorer`, `ase-client-viewer`, future GTK-based tools). Depends on `gtkmm-4.0` and `libadwaita-1` as system dependencies.

## Build

Built as part of the ASE root build when `ASE_BUILD_CLIENTS=ON`. Standalone:

```bash
cd adapter/ase-adp-gtk
cmake -B build -G Ninja
ninja -C build
```

## Adding new wrappers

When a client needs a gtkmm class not yet wrapped:

1. Add a header at `include/ase/adp/gtk/<class_name>.hpp` (or `include/ase/adp/adw/...` for libadwaita).
2. Add the implementation at `src/gtk_<class_name>.cpp` if non-trivial; header-only otherwise.
3. Update `include/ase/adp/gtk/gtk.hpp` (or `adw.hpp`) umbrella header.
4. Update `CMakeLists.txt` source list.
5. Update the wrapped surface table in this README.
