#pragma once

/**
 * @file        gesture.hpp
 * @brief       ClickGesture + KeyController + DragSource wrappers
 * @description Wrappers for gtkmm-4 input controllers. ClickGesture handles
 *              single/double click detection. KeyController handles keyboard
 *              shortcuts (raw key events). DragSource emits drag-and-drop data
 *              when the user starts dragging from a widget.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <gtkmm/gestureclick.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/dragsource.h>
#include <gdkmm/contentprovider.h>
#include <gdkmm/enums.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/** Mouse button enum, mirrors GDK_BUTTON_PRIMARY/SECONDARY/MIDDLE constants. */
enum class MouseButton {
    Primary = 1,
    Middle = 2,
    Secondary = 3,
};

/**
 * ClickGesture - detects mouse clicks (single and multi-click) on a widget.
 * Connect via on_released() to receive (n_press, x, y) triples. Pair with
 * set_button() to filter by mouse button.
 */
class ClickGesture {
public:
    static ClickGesture create();

    void set_button(MouseButton button);

    /** Connect a callable invoked when the click is released: void(int n_press, double x, double y). */
    template <typename Callback>
    void on_released(Callback&& callback) {
        connect_released_impl(
            sigc::slot<void(int, double, double)>(
                [cb = std::forward<Callback>(callback)](int n_press, double x, double y) { cb(n_press, x, y); }));
    }

    /** Internal: returns the gesture as a generic Gtk::EventController for add_controller(). */
    Glib::RefPtr<Gtk::EventController> native_controller() const noexcept;

private:
    explicit ClickGesture(Glib::RefPtr<Gtk::GestureClick> g) : m_gesture(std::move(g)) {}
    void connect_released_impl(sigc::slot<void(int, double, double)> slot);
    Glib::RefPtr<Gtk::GestureClick> m_gesture;
};

/** Modifier key bitfield, mirrors Gdk::ModifierType. */
enum class Modifier : unsigned int {
    None    = 0u,
    Shift   = 1u << 0,
    Control = 1u << 2,
    Alt     = 1u << 3,
};

inline Modifier operator|(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline Modifier operator&(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
}

inline bool has(Modifier value, Modifier flag) {
    return static_cast<unsigned int>(value & flag) != 0u;
}

/**
 * KeyController - listens for raw key press and release events on a widget.
 * The on_key_pressed callback returns true to consume the event, false to let
 * it propagate. The keyval is a GDK key constant (GDK_KEY_F5, etc.).
 */
class KeyController {
public:
    static KeyController create();

    /** Connect: bool(unsigned int keyval, unsigned int keycode, Modifier state). */
    template <typename Callback>
    void on_key_pressed(Callback&& callback) {
        connect_key_pressed_impl(
            sigc::slot<bool(unsigned int, unsigned int, Modifier)>(
                [cb = std::forward<Callback>(callback)](
                    unsigned int keyval, unsigned int keycode, Modifier state) -> bool {
                    return cb(keyval, keycode, state);
                }));
    }

    /** Internal: returns the controller as a generic Gtk::EventController. */
    Glib::RefPtr<Gtk::EventController> native_controller() const noexcept;

private:
    explicit KeyController(Glib::RefPtr<Gtk::EventControllerKey> c) : m_controller(std::move(c)) {}
    void connect_key_pressed_impl(sigc::slot<bool(unsigned int, unsigned int, Modifier)> slot);
    Glib::RefPtr<Gtk::EventControllerKey> m_controller;
};

/** Drag-and-drop action bitfield, mirrors Gdk::DragAction. */
enum class DragAction : unsigned int {
    None  = 0u,
    Copy  = 1u << 0,
    Move  = 1u << 1,
    Link  = 1u << 2,
};

/**
 * DragSource - emits drag-and-drop content when the user starts dragging from
 * a widget. The on_prepare callback returns the text payload to be dragged
 * (e.g., a file path). Empty return = drag is cancelled.
 */
class DragSource {
public:
    static DragSource create();

    void set_actions(DragAction actions);

    /** Connect: std::string(double x, double y) - return the drag payload as text. */
    template <typename Callback>
    void on_prepare(Callback&& callback) {
        connect_prepare_impl(
            sigc::slot<std::string(double, double)>(
                [cb = std::forward<Callback>(callback)](double x, double y) -> std::string {
                    return cb(x, y);
                }));
    }

    /**
     * Raw access to the underlying Gtk::DragSource. Use this when the feature
     * needs to return a composite ContentProvider (text/uri-list + text/plain
     * for file DnD) or install custom accumulator semantics that go beyond a
     * simple text payload. Feature code holds the RefPtr while the controller
     * is attached to a widget; the underlying source outlives individual
     * drag operations.
     */
    const Glib::RefPtr<Gtk::DragSource>& native() const noexcept { return m_source; }

    /** Internal: returns the drag source as a generic Gtk::EventController. */
    Glib::RefPtr<Gtk::EventController> native_controller() const noexcept;

private:
    explicit DragSource(Glib::RefPtr<Gtk::DragSource> d) : m_source(std::move(d)) {}
    void connect_prepare_impl(sigc::slot<std::string(double, double)> slot);
    Glib::RefPtr<Gtk::DragSource> m_source;
};

}  // namespace ase::adp::gtk
