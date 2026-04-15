#pragma once

/**
 * @file        window.hpp
 * @brief       Adwaita window stack: Window, ToolbarView, HeaderBar, ViewStack, ViewSwitcher
 * @description Adwaita is shipped as a GObject C library (libadwaita) without
 *              official gtkmm bindings. These wrappers call the C API directly
 *              and expose ASE-native value types holding raw GObject pointers.
 *              Lifetime: Adw widgets attach to a parent and are destroyed by
 *              the parent on close, just like Gtk widgets.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <adwaita.h>
#include <gtkmm/window.h>

#include <string>

namespace ase::adp::adw {

/**
 * Window - top-level standalone Adwaita window. Independent from any parent
 * Gtk::ApplicationWindow it may be transient for, with its own size, header,
 * and content. This is the modern non-deprecated AdwWindow (NOT AdwDialog,
 * which is constrained to its parent's viewport).
 */
class Window {
public:
    /** Create a new standalone AdwWindow. */
    static Window create();

    void set_title(const std::string& title);
    void set_default_size(int width, int height);
    void set_modal(bool modal);

    /** Set the parent window for transient positioning. */
    template <typename ParentWindow>
    void set_transient_for(ParentWindow& parent) {
        set_transient_for_native(GTK_WINDOW(parent.native_widget()->gobj()));
    }

    /** Install the content widget (typically a ToolbarView). */
    template <typename Content>
    void set_content(Content& content) {
        set_content_native(content.native_widget());
    }

    /** Show the window. */
    void present();

    /** Internal: returns the underlying AdwWindow pointer. */
    AdwWindow* native() const noexcept { return m_window; }

    /** Internal: returns the underlying widget pointer for transient_for/etc. */
    GtkWidget* native_widget() const noexcept { return GTK_WIDGET(m_window); }

private:
    explicit Window(AdwWindow* w) : m_window(w) {}
    void set_transient_for_native(GtkWindow* parent);
    void set_content_native(GtkWidget* content);
    AdwWindow* m_window;
};

/**
 * ToolbarView - vertical container with optional top and bottom bars (header
 * bars, toolbars) and a single content widget that fills the remaining area.
 * The standard root widget for a modern Adwaita window.
 */
class ToolbarView {
public:
    static ToolbarView create();

    template <typename Bar>
    void add_top_bar(Bar& bar) { add_top_bar_native(bar.native_widget()); }

    template <typename Bar>
    void add_bottom_bar(Bar& bar) { add_bottom_bar_native(bar.native_widget()); }

    template <typename Content>
    void set_content(Content& content) { set_content_native(content.native_widget()); }

    /** Internal: returns the underlying widget pointer for set_content. */
    GtkWidget* native_widget() const noexcept { return m_view; }

    AdwToolbarView* native() const noexcept { return ADW_TOOLBAR_VIEW(m_view); }

private:
    explicit ToolbarView(GtkWidget* v) : m_view(v) {}
    void add_top_bar_native(GtkWidget* bar);
    void add_bottom_bar_native(GtkWidget* bar);
    void set_content_native(GtkWidget* content);
    GtkWidget* m_view;
};

/**
 * HeaderBar - Adwaita header bar with standard window controls (close, minimise,
 * maximise) and an optional title widget. Goes inside ToolbarView::add_top_bar.
 */
class HeaderBar {
public:
    static HeaderBar create();

    template <typename TitleWidget>
    void set_title_widget(TitleWidget& w) { set_title_widget_native(w.native_widget()); }

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_header; }

    AdwHeaderBar* native() const noexcept { return ADW_HEADER_BAR(m_header); }

private:
    explicit HeaderBar(GtkWidget* h) : m_header(h) {}
    void set_title_widget_native(GtkWidget* w);
    GtkWidget* m_header;
};

/**
 * ViewStack - paged container that shows one of its children at a time. Pages
 * are added with add_titled_with_icon(), each gets a name (used by the switcher)
 * a title (visible label), and an icon name.
 */
class ViewStack {
public:
    static ViewStack create();

    /** Add a titled page with an icon. Page widgets must derive from any wrapper. */
    template <typename PageWidget>
    void add_titled_with_icon(
        PageWidget& page,
        const std::string& name,
        const std::string& title,
        const std::string& icon_name)
    {
        add_titled_with_icon_native(page.native_widget(), name, title, icon_name);
    }

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_stack; }

    AdwViewStack* native() const noexcept { return ADW_VIEW_STACK(m_stack); }

private:
    explicit ViewStack(GtkWidget* s) : m_stack(s) {}
    void add_titled_with_icon_native(
        GtkWidget* page, const std::string& name,
        const std::string& title, const std::string& icon_name);
    GtkWidget* m_stack;
};

/** ViewSwitcher policy - WIDE shows full labels, NARROW shows icons only. */
enum class ViewSwitcherPolicy { Narrow, Wide };

/**
 * ViewSwitcher - tab-like control that switches between pages of a ViewStack.
 * Typically placed in a HeaderBar's title position via set_title_widget().
 */
class ViewSwitcher {
public:
    static ViewSwitcher create();

    void set_stack(ViewStack& stack);
    void set_policy(ViewSwitcherPolicy policy);

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_switcher; }

    AdwViewSwitcher* native() const noexcept { return ADW_VIEW_SWITCHER(m_switcher); }

private:
    explicit ViewSwitcher(GtkWidget* s) : m_switcher(s) {}
    GtkWidget* m_switcher;
};

/** Adwaita color scheme - mirrors AdwColorScheme enum. */
enum class ColorScheme {
    Default,
    PreferLight,
    PreferDark,
    ForceLight,
    ForceDark,
};

/**
 * StyleManager - process-global Adwaita appearance manager. Use to force a
 * dark or light color scheme at startup. There is exactly one instance per
 * process (singleton in the C API, exposed as a free function here).
 */
namespace style_manager {

void init();
void set_color_scheme(ColorScheme scheme);

}  // namespace style_manager

}  // namespace ase::adp::adw
