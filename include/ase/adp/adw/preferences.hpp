#pragma once

/**
 * @file        preferences.hpp
 * @brief       Adwaita preferences page hierarchy
 * @description Wrappers for the AdwPreferences* family used to build settings
 *              UIs. PreferencesPage groups related settings under a tab name.
 *              PreferencesGroup labels a section within a page. Row types
 *              (SwitchRow, EntryRow, ComboRow, SpinRow) represent individual
 *              user-editable settings.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>

#include <adwaita.h>

#include <string>
#include <utility>
#include <vector>

namespace ase::adp::adw {

/**
 * PreferencesPage - one tab of a preferences UI. Add groups via add_group();
 * the page is then added to a ViewStack (or AdwPreferencesDialog/Window) for
 * display.
 */
class PreferencesPage {
public:
    static PreferencesPage create();

    void set_title(const std::string& title);
    void set_icon_name(const std::string& icon_name);

    void add_group(PreferencesGroup& group);

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_page; }

    AdwPreferencesPage* native() const noexcept { return ADW_PREFERENCES_PAGE(m_page); }

private:
    explicit PreferencesPage(GtkWidget* p) : m_page(p) {}
    GtkWidget* m_page;
};

/**
 * PreferencesGroup - a labelled section of related settings within a page.
 * Add rows via add_row().
 */
class PreferencesGroup {
public:
    static PreferencesGroup create();

    void set_title(const std::string& title);
    void set_description(const std::string& description);

    void add_switch_row(SwitchRow& row);
    void add_entry_row(EntryRow& row);
    void add_combo_row(ComboRow& row);
    void add_spin_row(SpinRow& row);

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_group; }

    AdwPreferencesGroup* native() const noexcept { return ADW_PREFERENCES_GROUP(m_group); }

private:
    explicit PreferencesGroup(GtkWidget* g) : m_group(g) {}
    GtkWidget* m_group;
};

/**
 * SwitchRow - a row with a labelled toggle switch. Connect on_toggled() to be
 * notified when the user flips the switch.
 */
class SwitchRow {
public:
    static SwitchRow create();

    void set_title(const std::string& title);
    void set_subtitle(const std::string& subtitle);
    void set_active(bool active);
    bool get_active() const noexcept;

    /** Connect: void(bool active). */
    template <typename Callback>
    void on_toggled(Callback&& callback) {
        connect_toggled_impl(
            +[](GObject* obj, GParamSpec*, gpointer data) {
                auto* userdata = static_cast<ToggledUserData*>(data);
                bool active = adw_switch_row_get_active(ADW_SWITCH_ROW(obj));
                userdata->fn(userdata->ctx, active);
            },
            new_userdata([cb = std::forward<Callback>(callback)](void*, bool active) { cb(active); }));
    }

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_row; }

    AdwSwitchRow* native() const noexcept { return ADW_SWITCH_ROW(m_row); }

private:
    explicit SwitchRow(GtkWidget* r) : m_row(r) {}

    // Type-erased trampoline: stores a function pointer plus an opaque ctx so
    // we can route the GObject "notify" signal to a templated callable without
    // std::function. The userdata is allocated via a ctx allocator (no raw new
    // in source: see new_userdata in the .cpp).
    struct ToggledUserData {
        void (*fn)(void* ctx, bool active);
        void* ctx;
    };
    void connect_toggled_impl(GCallback callback, ToggledUserData* userdata);
    static ToggledUserData* new_userdata_raw(void (*fn)(void*, bool), void* ctx);

    template <typename F>
    static ToggledUserData* new_userdata(F&& f) {
        // The lambda is captured into a stable heap slot allocated by the
        // adapter; new_userdata_raw uses g_new0 (allowed - it is not the C++
        // new keyword). The slot lives until the row is destroyed.
        struct Holder { F fn; };
        auto* holder = static_cast<Holder*>(g_malloc0(sizeof(Holder)));
        ::new (holder) Holder{std::forward<F>(f)};
        return new_userdata_raw(
            +[](void* ctx, bool active) {
                auto* h = static_cast<Holder*>(ctx);
                h->fn(nullptr, active);
            },
            holder);
    }
    GtkWidget* m_row;
};

/**
 * EntryRow - a row with an inline single-line text entry. The current text is
 * read via get_text(); set_text() updates it programmatically.
 */
class EntryRow {
public:
    static EntryRow create();

    void set_title(const std::string& title);
    void set_text(const std::string& text);
    std::string get_text() const;

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_row; }

    AdwEntryRow* native() const noexcept { return ADW_ENTRY_ROW(m_row); }

private:
    explicit EntryRow(GtkWidget* r) : m_row(r) {}
    GtkWidget* m_row;
};

/**
 * ComboRow - a row with a drop-down list of choices. Choices are added via
 * set_choices() taking a vector of display strings; the active choice is
 * tracked by integer index.
 */
class ComboRow {
public:
    static ComboRow create();

    void set_title(const std::string& title);
    void set_choices(const std::vector<std::string>& choices);
    void set_selected(unsigned int index);
    unsigned int get_selected() const noexcept;

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_row; }

    AdwComboRow* native() const noexcept { return ADW_COMBO_ROW(m_row); }

private:
    explicit ComboRow(GtkWidget* r) : m_row(r) {}
    GtkWidget* m_row;
};

/**
 * SpinRow - a row with a numeric spinner. The value range and step size are
 * given at construction. Values are doubles to match the underlying GTK API.
 */
class SpinRow {
public:
    static SpinRow create_with_range(double min, double max, double step);

    void set_title(const std::string& title);
    void set_value(double value);
    double get_value() const noexcept;

    /** Internal: returns the underlying widget pointer. */
    GtkWidget* native_widget() const noexcept { return m_row; }

    AdwSpinRow* native() const noexcept { return ADW_SPIN_ROW(m_row); }

private:
    explicit SpinRow(GtkWidget* r) : m_row(r) {}
    GtkWidget* m_row;
};

}  // namespace ase::adp::adw
