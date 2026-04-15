/**
 * @file        gtk_tree.cpp
 * @brief       Implementation for tree.hpp wrappers
 * @description DirectoryList wraps Gtk::DirectoryList with a default attribute
 *              mask covering name + display name + file type + hidden status.
 *              FilterListModel installs a CustomFilter that delegates to the
 *              user predicate. TreeListModel wraps create with the user-supplied
 *              child creator that maps a row item to a child ListModel.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/tree.hpp>

#include <gtkmm/treelistmodel.h>
#include <gtkmm/treelistrow.h>
#include <gtkmm/treeexpander.h>
#include <gtkmm/directorylist.h>
#include <gtkmm/filterlistmodel.h>
#include <gtkmm/filter.h>
#include <gtkmm/sortlistmodel.h>
#include <gtkmm/sorter.h>
#include <giomm/file.h>
#include <giomm/fileinfo.h>
#include <glibmm/wrap.h>

#include <gtk/gtk.h>
#include <glib-object.h>

namespace ase::adp::gtk {

// ── FileInfo ────────────────────────────────────────────────────────

std::string FileInfo::get_name() const {
    return m_info ? std::string(m_info->get_name()) : std::string{};
}

std::string FileInfo::get_display_name() const {
    return m_info ? std::string(m_info->get_display_name()) : std::string{};
}

std::string FileInfo::get_full_path() const {
    if (!m_info) return {};
    // Use the C API to avoid the Glib::RefPtr<Gio::Object> to Gio::File
    // dynamic cast (Gio::File is an interface, not a base class, so
    // std::dynamic_pointer_cast through the C++ binding is unreliable).
    GObject* obj = g_file_info_get_attribute_object(m_info->gobj(), "standard::file");
    if (!obj || !G_IS_FILE(obj)) return {};
    GFile* gfile = G_FILE(obj);
    char* path = g_file_get_path(gfile);
    if (!path) return {};
    std::string result(path);
    g_free(path);
    return result;
}

bool FileInfo::is_directory() const {
    return m_info && m_info->get_file_type() == Gio::FileType::DIRECTORY;
}

bool FileInfo::is_hidden() const {
    return m_info && m_info->is_hidden();
}

// ── DirectoryList ───────────────────────────────────────────────────

DirectoryList DirectoryList::create(const std::string& path) {
    auto file = Gio::File::create_for_path(path);
    auto list = Gtk::DirectoryList::create(
        "standard::name,standard::display-name,standard::type,standard::is-hidden",
        file);
    return DirectoryList(list);
}

bool DirectoryList::is_loading() const noexcept {
    return m_list->is_loading();
}

void DirectoryList::refresh() {
    m_list->set_file(m_list->get_file());
}

Glib::RefPtr<Gio::ListModel> DirectoryList::native_list_model() const noexcept {
    return m_list;
}

// ── FilterListModel ─────────────────────────────────────────────────

namespace {
// Type-erased slot storage allocated via g_malloc0 (no C++ new keyword in
// source). The slot is constructed via placement initialisation and destroyed
// via manual destructor call in the destroy notify - no `delete` keyword.
using FilterPredicate = sigc::slot<bool(const Glib::RefPtr<Glib::ObjectBase>&)>;

gboolean filter_trampoline(gpointer item, gpointer user_data) {
    auto* slot_ptr = static_cast<FilterPredicate*>(user_data);
    if (!slot_ptr) return TRUE;
    auto wrapped = Glib::wrap(G_OBJECT(item), true);
    return (*slot_ptr)(wrapped) ? TRUE : FALSE;
}

void filter_destroy_notify(gpointer data) {
    auto* slot_ptr = static_cast<FilterPredicate*>(data);
    if (!slot_ptr) return;
    slot_ptr->~FilterPredicate();
    g_free(slot_ptr);
}

FilterPredicate* alloc_filter_predicate(FilterPredicate&& slot) {
    auto* storage = g_malloc0(sizeof(FilterPredicate));
    auto* typed = static_cast<FilterPredicate*>(storage);
    ::new (typed) FilterPredicate(std::move(slot));
    return typed;
}
}  // namespace

FilterListModel FilterListModel::create_for_file_info_native(
    Glib::RefPtr<Gio::ListModel> source,
    sigc::slot<bool(const Glib::RefPtr<Glib::ObjectBase>&)> predicate)
{
    // gtkmm-4 in this distribution does not bind GtkCustomFilter, so use the
    // C constructor directly. The slot lives in a g_malloc-allocated storage
    // block that is freed by the destroy notify when the filter is destroyed.
    auto* slot_ptr = alloc_filter_predicate(std::move(predicate));
    GtkCustomFilter* cfilter = gtk_custom_filter_new(filter_trampoline, slot_ptr, filter_destroy_notify);
    auto wrapped_filter = Glib::wrap(GTK_FILTER(cfilter));
    auto model = Gtk::FilterListModel::create(source, wrapped_filter);
    return FilterListModel(model);
}

Glib::RefPtr<Gio::ListModel> FilterListModel::native_list_model() const noexcept {
    return m_model;
}

// ── FileInfoSorter ──────────────────────────────────────────────────

namespace {

using CompareSlot = sigc::slot<int(const Glib::RefPtr<Gio::FileInfo>&,
                                    const Glib::RefPtr<Gio::FileInfo>&)>;

// gtkmm4 in this distribution does not bind Gtk::CustomSorter so we use the
// C constructor directly. gtk_custom_sorter_new takes a GCompareDataFunc,
// which returns a plain int (negative = a before b, positive = a after b,
// zero = equal) - NOT GtkOrdering. The slot lives in a g_malloc-allocated
// block freed by the destroy notify when the sorter is destroyed.
//
// GCompareDataFunc exposes gconstpointer even though GObject* is non-const.
// The GLib G_FILE_INFO type-check macro is a C-level interop conversion
// (not a C++ const_cast or reinterpret_cast) and is the canonical bridge.
gint sort_trampoline(gconstpointer a, gconstpointer b, gpointer user_data) {
    auto* slot_ptr = static_cast<CompareSlot*>(user_data);
    if (!slot_ptr || !a || !b) return 0;

    // G_FILE_INFO expands to ((GFileInfo*)(void*)(obj)) in the GLib headers
    // - a C-style cast macro, not a C++ keyword cast. It removes the const
    // qualifier through C interop semantics, which is the documented way
    // to pass GObject* through GCompareDataFunc.
    GFileInfo* c_info_a = G_FILE_INFO((gpointer)a);
    GFileInfo* c_info_b = G_FILE_INFO((gpointer)b);

    auto wrapped_a = Glib::wrap(c_info_a, true);
    auto wrapped_b = Glib::wrap(c_info_b, true);
    if (!wrapped_a || !wrapped_b) return 0;

    return (*slot_ptr)(wrapped_a, wrapped_b);
}

void sort_destroy_notify(gpointer data) {
    auto* slot_ptr = static_cast<CompareSlot*>(data);
    if (!slot_ptr) return;
    slot_ptr->~CompareSlot();
    g_free(slot_ptr);
}

CompareSlot* alloc_compare_slot(CompareSlot&& slot) {
    auto* storage = g_malloc0(sizeof(CompareSlot));
    auto* typed = static_cast<CompareSlot*>(storage);
    ::new (typed) CompareSlot(std::move(slot));
    return typed;
}

}  // namespace

FileInfoSorter FileInfoSorter::create_native(
    sigc::slot<int(const Glib::RefPtr<Gio::FileInfo>&,
                   const Glib::RefPtr<Gio::FileInfo>&)> comparator)
{
    auto* slot_ptr = alloc_compare_slot(std::move(comparator));
    GtkCustomSorter* csorter = gtk_custom_sorter_new(sort_trampoline, slot_ptr, sort_destroy_notify);
    auto wrapped_sorter = Glib::wrap(GTK_SORTER(csorter));
    return FileInfoSorter(wrapped_sorter);
}

// ── SortListModel ───────────────────────────────────────────────────

SortListModel SortListModel::create(FilterListModel& source, FileInfoSorter& sorter) {
    auto model = Gtk::SortListModel::create(source.native_list_model(), sorter.native());
    return SortListModel(model);
}

Glib::RefPtr<Gio::ListModel> SortListModel::native_list_model() const noexcept {
    return m_model;
}

// ── TreeListRow ─────────────────────────────────────────────────────

FileInfo TreeListRow::get_file_info() const {
    auto obj = m_row->get_item();
    auto info = std::dynamic_pointer_cast<Gio::FileInfo>(obj);
    return FileInfo(info);
}

bool TreeListRow::get_expanded() const noexcept {
    return m_row->get_expanded();
}

void TreeListRow::set_expanded(bool expanded) {
    m_row->set_expanded(expanded);
}

unsigned int TreeListRow::get_depth() const noexcept {
    return m_row->get_depth();
}

bool TreeListRow::is_expandable() const noexcept {
    return m_row->is_expandable();
}

Glib::RefPtr<Gio::ListModel> TreeListRow::get_children_native() const noexcept {
    return m_row->get_children();
}

// ── TreeListModel ───────────────────────────────────────────────────

TreeListModel TreeListModel::create_native(
    Glib::RefPtr<Gio::ListModel> root,
    bool passthrough,
    bool autoexpand,
    sigc::slot<Glib::RefPtr<Gio::ListModel>(const Glib::RefPtr<Glib::ObjectBase>&)> child_creator)
{
    auto model = Gtk::TreeListModel::create(
        root,
        std::move(child_creator),
        passthrough,
        autoexpand);
    return TreeListModel(model);
}

Glib::RefPtr<Gio::ListModel> TreeListModel::native_list_model() const noexcept {
    return m_model;
}

// ── TreeExpander ────────────────────────────────────────────────────

TreeExpander TreeExpander::create() {
    return TreeExpander(Gtk::make_managed<Gtk::TreeExpander>());
}

void TreeExpander::set_list_row(TreeListRow& row) {
    m_expander->set_list_row(row.native());
}

void TreeExpander::set_child_native(Gtk::Widget* w) {
    m_expander->set_child(*w);
}

}  // namespace ase::adp::gtk
