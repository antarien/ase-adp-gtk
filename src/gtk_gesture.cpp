/**
 * @file        gtk_gesture.cpp
 * @brief       Implementation for gesture.hpp wrappers
 * @description ClickGesture, KeyController, and DragSource each wrap their
 *              gtkmm equivalents and expose a single connect_*_impl method that
 *              forwards a sigc::slot to the underlying signal. native_controller
 *              upcasts the specific controller type to the base Gtk::EventController
 *              for use with widget add_controller().
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/gesture.hpp>

#include <gtkmm/gestureclick.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/eventcontroller.h>
#include <gtkmm/dragsource.h>
#include <gdkmm/contentprovider.h>
#include <gdkmm/enums.h>
#include <glibmm/bytes.h>

namespace ase::adp::gtk {

namespace {

Modifier from_gdk(Gdk::ModifierType state) {
    unsigned int v = 0;
    if ((state & Gdk::ModifierType::SHIFT_MASK) != Gdk::ModifierType()) {
        v |= static_cast<unsigned int>(Modifier::Shift);
    }
    if ((state & Gdk::ModifierType::CONTROL_MASK) != Gdk::ModifierType()) {
        v |= static_cast<unsigned int>(Modifier::Control);
    }
    if ((state & Gdk::ModifierType::ALT_MASK) != Gdk::ModifierType()) {
        v |= static_cast<unsigned int>(Modifier::Alt);
    }
    return static_cast<Modifier>(v);
}

Gdk::DragAction to_gdk_drag_action(DragAction actions) {
    Gdk::DragAction result = Gdk::DragAction();
    auto v = static_cast<unsigned int>(actions);
    if (v & static_cast<unsigned int>(DragAction::Copy)) result |= Gdk::DragAction::COPY;
    if (v & static_cast<unsigned int>(DragAction::Move)) result |= Gdk::DragAction::MOVE;
    if (v & static_cast<unsigned int>(DragAction::Link)) result |= Gdk::DragAction::LINK;
    return result;
}

}  // namespace

// ── ClickGesture ────────────────────────────────────────────────────

ClickGesture ClickGesture::create() {
    return ClickGesture(Gtk::GestureClick::create());
}

void ClickGesture::set_button(MouseButton button) {
    m_gesture->set_button(static_cast<unsigned int>(button));
}

void ClickGesture::connect_released_impl(sigc::slot<void(int, double, double)> slot) {
    m_gesture->signal_released().connect(std::move(slot));
}

Glib::RefPtr<Gtk::EventController> ClickGesture::native_controller() const noexcept {
    return m_gesture;
}

// ── KeyController ───────────────────────────────────────────────────

KeyController KeyController::create() {
    return KeyController(Gtk::EventControllerKey::create());
}

void KeyController::connect_key_pressed_impl(
    sigc::slot<bool(unsigned int, unsigned int, Modifier)> slot)
{
    m_controller->signal_key_pressed().connect(
        [slot](guint keyval, guint keycode, Gdk::ModifierType state) -> bool {
            return slot(keyval, keycode, from_gdk(state));
        }, false);
}

Glib::RefPtr<Gtk::EventController> KeyController::native_controller() const noexcept {
    return m_controller;
}

// ── DragSource ──────────────────────────────────────────────────────

DragSource DragSource::create() {
    return DragSource(Gtk::DragSource::create());
}

void DragSource::set_actions(DragAction actions) {
    m_source->set_actions(to_gdk_drag_action(actions));
}

void DragSource::connect_prepare_impl(sigc::slot<std::string(double, double)> slot) {
    m_source->signal_prepare().connect(
        [slot](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> {
            std::string payload = slot(x, y);
            if (payload.empty()) return {};
            auto bytes = Glib::Bytes::create(payload.data(), payload.size());
            return Gdk::ContentProvider::create("text/plain", bytes);
        }, false);
}

Glib::RefPtr<Gtk::EventController> DragSource::native_controller() const noexcept {
    return m_source;
}

}  // namespace ase::adp::gtk
