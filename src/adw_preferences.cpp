/**
 * @file        adw_preferences.cpp
 * @brief       Implementation for adw/preferences.hpp wrappers
 * @description Wraps the AdwPreferences* C API. Each row constructor calls the
 *              corresponding C creator (adw_switch_row_new, etc.) which returns
 *              a GtkWidget*. The lookup-table-free SwitchRow user-data slot is
 *              allocated via g_malloc0 plus placement initialisation - no raw
 *              C++ new is used in this source file.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/adw/preferences.hpp>

#include <adwaita.h>
#include <gtk/gtk.h>

namespace ase::adp::adw {

// ── PreferencesPage ─────────────────────────────────────────────────

PreferencesPage PreferencesPage::create() {
    return PreferencesPage(adw_preferences_page_new());
}

void PreferencesPage::set_title(const std::string& title) {
    adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_page), title.c_str());
}

void PreferencesPage::set_icon_name(const std::string& icon_name) {
    adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_page), icon_name.c_str());
}

void PreferencesPage::add_group(PreferencesGroup& group) {
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(m_page),
                             ADW_PREFERENCES_GROUP(group.native_widget()));
}

// ── PreferencesGroup ────────────────────────────────────────────────

PreferencesGroup PreferencesGroup::create() {
    return PreferencesGroup(adw_preferences_group_new());
}

void PreferencesGroup::set_title(const std::string& title) {
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(m_group), title.c_str());
}

void PreferencesGroup::set_description(const std::string& description) {
    adw_preferences_group_set_description(ADW_PREFERENCES_GROUP(m_group), description.c_str());
}

void PreferencesGroup::add_switch_row(SwitchRow& row) {
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_group), row.native_widget());
}

void PreferencesGroup::add_entry_row(EntryRow& row) {
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_group), row.native_widget());
}

void PreferencesGroup::add_combo_row(ComboRow& row) {
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_group), row.native_widget());
}

void PreferencesGroup::add_spin_row(SpinRow& row) {
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_group), row.native_widget());
}

// ── SwitchRow ───────────────────────────────────────────────────────

SwitchRow SwitchRow::create() {
    return SwitchRow(adw_switch_row_new());
}

void SwitchRow::set_title(const std::string& title) {
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_row), title.c_str());
}

void SwitchRow::set_subtitle(const std::string& subtitle) {
    adw_action_row_set_subtitle(ADW_ACTION_ROW(m_row), subtitle.c_str());
}

void SwitchRow::set_active(bool active) {
    adw_switch_row_set_active(ADW_SWITCH_ROW(m_row), active ? TRUE : FALSE);
}

bool SwitchRow::get_active() const noexcept {
    return adw_switch_row_get_active(ADW_SWITCH_ROW(m_row));
}

void SwitchRow::connect_toggled_impl(GCallback callback, ToggledUserData* userdata) {
    g_signal_connect(m_row, "notify::active", callback, userdata);
}

SwitchRow::ToggledUserData* SwitchRow::new_userdata_raw(void (*fn)(void*, bool), void* ctx) {
    auto* ud = static_cast<ToggledUserData*>(g_malloc0(sizeof(ToggledUserData)));
    ud->fn = fn;
    ud->ctx = ctx;
    return ud;
}

// ── EntryRow ────────────────────────────────────────────────────────

EntryRow EntryRow::create() {
    return EntryRow(adw_entry_row_new());
}

void EntryRow::set_title(const std::string& title) {
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_row), title.c_str());
}

void EntryRow::set_text(const std::string& text) {
    gtk_editable_set_text(GTK_EDITABLE(m_row), text.c_str());
}

std::string EntryRow::get_text() const {
    const char* s = gtk_editable_get_text(GTK_EDITABLE(m_row));
    return s ? std::string(s) : std::string{};
}

// ── ComboRow ────────────────────────────────────────────────────────

ComboRow ComboRow::create() {
    return ComboRow(adw_combo_row_new());
}

void ComboRow::set_title(const std::string& title) {
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_row), title.c_str());
}

void ComboRow::set_choices(const std::vector<std::string>& choices) {
    GtkStringList* list = gtk_string_list_new(nullptr);
    for (const auto& choice : choices) {
        gtk_string_list_append(list, choice.c_str());
    }
    adw_combo_row_set_model(ADW_COMBO_ROW(m_row), G_LIST_MODEL(list));
}

void ComboRow::set_selected(unsigned int index) {
    adw_combo_row_set_selected(ADW_COMBO_ROW(m_row), index);
}

unsigned int ComboRow::get_selected() const noexcept {
    return adw_combo_row_get_selected(ADW_COMBO_ROW(m_row));
}

// ── SpinRow ─────────────────────────────────────────────────────────

SpinRow SpinRow::create_with_range(double min, double max, double step) {
    return SpinRow(adw_spin_row_new_with_range(min, max, step));
}

void SpinRow::set_title(const std::string& title) {
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_row), title.c_str());
}

void SpinRow::set_value(double value) {
    adw_spin_row_set_value(ADW_SPIN_ROW(m_row), value);
}

double SpinRow::get_value() const noexcept {
    return adw_spin_row_get_value(ADW_SPIN_ROW(m_row));
}

}  // namespace ase::adp::adw
