/**
 * ASE GTK ADAPTER - FOLDER PICKER
 *
 * @file        adw_folder_picker.cpp
 * @brief       Implementation for adw/folder_picker.hpp
 * @description AdwWindow + GtkListBox + ase-fileio. No GtkFileDialog, no GtkFileChooser,
 *              no GtkPathBar, no portal dependency. Every widget is created and managed here.
 *
 *              THREE HOUSE FORMS, each taken from a measured place rather than invented:
 *
 *              1. PATHS through ase-fileio (L0): path_exists, is_directory, parent_of,
 *                 path_join, absolute_normalized, list_dir. The path library of the standard
 *                 library is barred tree-wide. absolute_normalized takes the reference
 *                 directory as a PARAMETER, not from process state; it comes from
 *                 fileio::current_dir here, so the dependency stands visibly in this file.
 *              2. STATE through g_malloc0 plus construction in place, released through the
 *                 explicit destructor call and g_free. The state outlives show() and belongs
 *                 to the window, not to the stack.
 *              3. CALLBACK as a function pointer plus user_data, like every GTK callback.
 *
 *              WHAT LISTING MUST NOT LOSE: list_dir writes into a buffer of fixed size and
 *              reports overflow by returning exactly the capacity. A folder with more entries
 *              would otherwise show silently cut short; the loop below doubles and asks again.
 *              Unreadable directories return 0.
 *
 * @module      ase-adp-gtk
 * @layer       5 (Adapter)
 * @category    render/ui/widget
 * @usecase     usecase:101129 - choose a folder
 * @created     2026-10-19
 * @modified    2026-10-19
 * @version     00.00.01.00001 [seed]
 *
 * Copyright (c) Jan Ohlmann. All rights reserved.
 * Licensed to ANTARES SIMULACAO LTDA. Use is governed by LICENSE.md and PJV.md in this repository.
 * SPDX-License-Identifier: LicenseRef-ASE-LIC
 *
 * ADAPTER COMPLIANCE
 *
 * [ ] English only - identifiers, comments, messages
 * [ ] META fields complete, @layer 5 (Section 0)
 * [ ] Layer dependencies correct (Section 11)
 * [ ] Headings are bare capital lines, cuts are empty lines (Section 0.1)
 * [ ] Form defined in INST_ASE_LINT.md Section 22
 */

#include <ase/adp/adw/folder_picker.hpp>

#include <ase/adp/adw/cockpit.hpp>

#include "design_tokens.hpp"

#include <ase/containers/vector.hpp>
#include <ase/fileio/directory.hpp>
#include <ase/fileio/path.hpp>

#include <adwaita.h>
#include <gtk/gtk.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace ase::adp::adw::folder_picker {

namespace t = ase::theme;

namespace {

/// Entries per list_dir call before the buffer is grown.
constexpr uint32_t INITIAL_DIR_CAPACITY = 128u;

struct PickerState {
    GtkWidget*  window      = nullptr;
    GtkWidget*  path_entry  = nullptr;
    GtkWidget*  listbox     = nullptr;
    GtkWidget*  open_button = nullptr;
    std::string current;
    SelectedFn  on_selected = nullptr;
    void*       user_data   = nullptr;
};

void rebuild_listbox(PickerState* state);

/// Make a path absolute against the process working directory and collapse "." and "..".
/// The working directory is read here rather than deep inside the path helper so the
/// dependency on process state is visible at the one place that has it.
std::string to_absolute(const std::string& path) {
    char           cwd[fileio::DIR_PATH_MAX];
    const uint32_t len = fileio::current_dir(cwd, fileio::DIR_PATH_MAX);
    return fileio::absolute_normalized(path, len > 0u ? std::string(cwd) : std::string("/"));
}

std::string sanitise_initial(const std::string& start) {
    if (!start.empty() && fileio::path_exists(start) && fileio::is_directory(start)) {
        return to_absolute(start);
    }
    if (const char* home = std::getenv("HOME"); home && *home) return home;
    return "/";
}

void navigate_to(PickerState* state, const std::string& path) {
    if (!fileio::path_exists(path) || !fileio::is_directory(path)) return;
    state->current = to_absolute(path);
    gtk_editable_set_text(GTK_EDITABLE(state->path_entry), state->current.c_str());
    rebuild_listbox(state);
}

void on_row_activated_cb(GtkListBox*, GtkListBoxRow* row, gpointer user_data) {
    auto* state = static_cast<PickerState*>(user_data);
    if (!row) return;
    const char* name = static_cast<const char*>(g_object_get_data(G_OBJECT(row), "ase-folder-name"));
    if (!name) return;
    navigate_to(state, fileio::path_join(state->current, name));
}

void on_path_entry_activate_cb(GtkEntry* entry, gpointer user_data) {
    auto* state = static_cast<PickerState*>(user_data);
    const char* text = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (text && *text) navigate_to(state, text);
}

void on_up_clicked_cb(GtkButton*, gpointer user_data) {
    auto*       state = static_cast<PickerState*>(user_data);
    std::string parent = fileio::parent_of(state->current);
    if (parent.empty()) parent = "/";
    navigate_to(state, parent);
}

void on_home_clicked_cb(GtkButton*, gpointer user_data) {
    auto* state = static_cast<PickerState*>(user_data);
    if (const char* home = std::getenv("HOME"); home && *home) navigate_to(state, home);
}

void on_open_clicked_cb(GtkButton*, gpointer user_data) {
    auto* state = static_cast<PickerState*>(user_data);
    // Use whatever the entry currently shows (matches user expectation if
    // they typed a path manually rather than navigating).
    const char* text = gtk_editable_get_text(GTK_EDITABLE(state->path_entry));
    std::string final_path = text && *text ? std::string(text) : state->current;
    if (!fileio::path_exists(final_path) || !fileio::is_directory(final_path)) return;
    if (state->on_selected) state->on_selected(final_path, state->user_data);
    gtk_window_close(GTK_WINDOW(state->window));
}

void on_cancel_clicked_cb(GtkButton*, gpointer user_data) {
    auto* state = static_cast<PickerState*>(user_data);
    gtk_window_close(GTK_WINDOW(state->window));
}

void on_window_destroy_cb(GtkWidget*, gpointer user_data) {
    // PickerState was constructed in place into a g_malloc0 block in show(), so the
    // destructor runs by hand before the block goes back to GLib. The std::string member
    // owns a heap buffer - skipping this call would leak it.
    auto* state = static_cast<PickerState*>(user_data);
    if (!state) return;
    state->~PickerState();
    g_free(state);
}

GtkWidget* build_row(const std::string& name) {
    GtkWidget* row = gtk_list_box_row_new();
    gtk_widget_add_css_class(row, "ase-cfg-app-row");

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, t::space_sm);

    GtkWidget* label = gtk_label_new(name.c_str());
    gtk_widget_add_css_class(label, "ase-cfg-app-name");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(hbox), label);

    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);

    char* name_dup = g_strdup(name.c_str());
    g_object_set_data_full(G_OBJECT(row), "ase-folder-name", name_dup, g_free);
    return row;
}

gboolean on_escape_key_cb(GtkEventControllerKey*, guint keyval, guint,
                          GdkModifierType, gpointer user_data)
{
    if (keyval == GDK_KEY_Escape) {
        gtk_window_close(GTK_WINDOW(user_data));
        return TRUE;
    }
    return FALSE;
}

void rebuild_listbox(PickerState* state) {
    if (!state || !state->listbox) return;

    GtkWidget* child = gtk_widget_get_first_child(state->listbox);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(state->listbox), child);
        child = next;
    }

    ase::containers::Vector<fileio::DirEntry> buffer(INITIAL_DIR_CAPACITY);

    // "written == capacity" is the overflow signal, not an error, so the only way to tell
    // "exactly full" from "truncated" is to ask again with more room.
    uint32_t found = 0u;
    for (;;) {
        const uint32_t capacity = static_cast<uint32_t>(buffer.size());
        found = fileio::list_dir(state->current.c_str(),
                                 static_cast<uint32_t>(state->current.size()),
                                 buffer.data(), capacity);
        if (found < capacity) break;
        buffer.resize(buffer.size() * 2u);
    }

    ase::containers::Vector<std::string> dirs;
    for (uint32_t i = 0u; i < found; ++i) {
        // is_dir follows a link, and that is what a picker wants: a link to a folder is a
        // folder to the user.
        if (buffer[i].is_dir) dirs.push_back(std::string(buffer[i].name));
    }

    std::sort(dirs.begin(), dirs.end(),
              [](const std::string& a, const std::string& b) {
                  std::string la = a, lb = b;
                  std::transform(la.begin(), la.end(), la.begin(), ::tolower);
                  std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
                  return la < lb;
              });

    for (const auto& d : dirs) {
        gtk_list_box_append(GTK_LIST_BOX(state->listbox), build_row(d));
    }
}

}  // namespace

void show(GtkWindow* parent,
          const std::string& start_path,
          SelectedFn on_selected,
          void* user_data)
{
    // PickerState outlives this function: it is owned by the window and released in
    // on_window_destroy_cb. Allocate via GLib and construct in place so the GTK widget
    // lifetime, not the C++ stack, governs its destruction.
    auto* state = static_cast<PickerState*>(g_malloc0(sizeof(PickerState)));
    new (state) PickerState();
    state->current     = sanitise_initial(start_path);
    state->on_selected = on_selected;
    state->user_data   = user_data;

    GtkWidget* window = adw_window_new();
    state->window = window;
    gtk_widget_add_css_class(window, "ase-cfg-window");
    gtk_window_set_title(GTK_WINDOW(window), "Choose a folder");
    gtk_window_set_default_size(GTK_WINDOW(window),
        t::space_2xl * 14,   // ~672 px
        t::space_2xl * 12);  // ~576 px
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(window), parent);

    // Escape closes the picker.
    GtkEventController* esc = gtk_event_controller_key_new();
    g_signal_connect(esc, "key-pressed", G_CALLBACK(on_escape_key_cb), window);
    gtk_widget_add_controller(window, esc);

    GtkWidget* toolbar = adw_toolbar_view_new();
    GtkWidget* header  = adw_header_bar_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar), header);

    GtkWidget* body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(body, "ase-cfg-pane");

    // Section strip header - the strip the settings pages head their sections with.
    gtk_box_append(GTK_BOX(body), cockpit::make_section_strip("CHOOSE FOLDER"));

    // Top row: parent, Home, path entry.
    GtkWidget* nav_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, t::space_sm);
    gtk_widget_set_margin_start (nav_row, t::space_md);
    gtk_widget_set_margin_end   (nav_row, t::space_md);
    gtk_widget_set_margin_top   (nav_row, t::space_md);
    gtk_widget_set_margin_bottom(nav_row, t::space_sm);

    GtkWidget* up_btn = gtk_button_new_with_label("↑");
    gtk_widget_set_tooltip_text(up_btn, "Parent directory");
    g_signal_connect(up_btn, "clicked", G_CALLBACK(on_up_clicked_cb), state);
    gtk_box_append(GTK_BOX(nav_row), up_btn);

    GtkWidget* home_btn = gtk_button_new_with_label("HOME");
    g_signal_connect(home_btn, "clicked", G_CALLBACK(on_home_clicked_cb), state);
    gtk_box_append(GTK_BOX(nav_row), home_btn);

    GtkWidget* path_entry = gtk_entry_new();
    state->path_entry = path_entry;
    gtk_widget_set_hexpand(path_entry, TRUE);
    gtk_widget_set_halign(path_entry, GTK_ALIGN_FILL);
    gtk_editable_set_text(GTK_EDITABLE(path_entry), state->current.c_str());
    g_signal_connect(path_entry, "activate", G_CALLBACK(on_path_entry_activate_cb), state);
    gtk_box_append(GTK_BOX(nav_row), path_entry);

    gtk_box_append(GTK_BOX(body), nav_row);

    // Scrolled directory listing.
    GtkWidget* scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* listbox = gtk_list_box_new();
    state->listbox = listbox;
    gtk_widget_add_css_class(listbox, "ase-cfg-window");
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_BROWSE);
    g_signal_connect(listbox, "row-activated", G_CALLBACK(on_row_activated_cb), state);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), listbox);
    gtk_box_append(GTK_BOX(body), scroll);

    // Footer: Cancel and Open.
    GtkWidget* footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, t::space_sm);
    gtk_widget_set_margin_start (footer, t::space_md);
    gtk_widget_set_margin_end   (footer, t::space_md);
    gtk_widget_set_margin_top   (footer, t::space_sm);
    gtk_widget_set_margin_bottom(footer, t::space_md);
    gtk_widget_set_halign(footer, GTK_ALIGN_END);

    GtkWidget* cancel_btn = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_clicked_cb), state);
    gtk_box_append(GTK_BOX(footer), cancel_btn);

    GtkWidget* open_btn = gtk_button_new_with_label("Open");
    state->open_button = open_btn;
    gtk_widget_add_css_class(open_btn, "suggested-action");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_clicked_cb), state);
    gtk_box_append(GTK_BOX(footer), open_btn);

    gtk_box_append(GTK_BOX(body), footer);

    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar), body);
    adw_window_set_content(ADW_WINDOW(window), toolbar);

    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy_cb), state);

    rebuild_listbox(state);
    gtk_window_present(GTK_WINDOW(window));
}

}  // namespace ase::adp::adw::folder_picker
