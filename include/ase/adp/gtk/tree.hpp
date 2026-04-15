#pragma once

/**
 * @file        tree.hpp
 * @brief       TreeListModel + TreeListRow + TreeExpander + DirectoryList + FilterListModel
 * @description Wrappers for hierarchical list models. TreeListModel turns a flat
 *              ListModel into a tree by calling a child-creator for each row.
 *              DirectoryList is the gtkmm-native enumeration of a filesystem
 *              directory and replaces the explorer's custom Glib::Object subclass
 *              entirely. FilterListModel applies a predicate to filter entries.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>
#include <ase/adp/gtk/widget.hpp>  // for detail::widget_* helpers

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
#include <giomm/listmodel.h>
#include <glibmm/refptr.h>

#include <string>
#include <utility>

namespace ase::adp::gtk {

/**
 * FileInfo - wraps Gio::FileInfo, the metadata object returned by DirectoryList.
 * Exposes the file name, full URI, and type (regular file vs directory).
 */
class FileInfo {
public:
    explicit FileInfo(Glib::RefPtr<Gio::FileInfo> info) : m_info(std::move(info)) {}

    std::string get_name() const;
    std::string get_display_name() const;
    /** Returns the full absolute filesystem path of this entry. */
    std::string get_full_path() const;
    bool is_directory() const;
    bool is_hidden() const;

    /** Internal: returns the underlying Gio::FileInfo refptr. */
    const Glib::RefPtr<Gio::FileInfo>& native() const noexcept { return m_info; }

private:
    Glib::RefPtr<Gio::FileInfo> m_info;
};

/**
 * DirectoryList - gtkmm-native ListModel that enumerates the contents of a
 * filesystem directory. Items are Gio::FileInfo objects. Replaces the custom
 * Glib::Object subclass approach (Gio::ListStore<FileEntry>) entirely - clients
 * get a clean list of FileInfos without needing to register a GObject type.
 */
class DirectoryList {
public:
    /** Create a new DirectoryList for the given absolute filesystem path. */
    static DirectoryList create(const std::string& path);

    /** Returns true while the underlying directory is being enumerated. */
    bool is_loading() const noexcept;

    /** Force a reload of the directory contents. */
    void refresh();

    /** Internal: returns the underlying Gio::ListModel refptr (for SelectionModel/etc). */
    Glib::RefPtr<Gio::ListModel> native_list_model() const noexcept;

    /** Internal: returns the underlying Gtk::DirectoryList refptr. */
    const Glib::RefPtr<Gtk::DirectoryList>& native() const noexcept { return m_list; }

private:
    explicit DirectoryList(Glib::RefPtr<Gtk::DirectoryList> l) : m_list(std::move(l)) {}
    Glib::RefPtr<Gtk::DirectoryList> m_list;
};

/**
 * FilterListModel - wraps another ListModel and applies a predicate to filter
 * out unwanted items. The predicate is a template callable: bool(FileInfo&).
 * Used by the explorer to exclude .git, build/, node_modules, etc.
 */
class FilterListModel {
public:
    /** Create a FilterListModel wrapping the given source with a FileInfo predicate. */
    template <typename Predicate>
    static FilterListModel create_for_file_info(DirectoryList& source, Predicate&& predicate) {
        return create_for_file_info_native(
            source.native_list_model(),
            sigc::slot<bool(const Glib::RefPtr<Glib::ObjectBase>&)>(
                [pred = std::forward<Predicate>(predicate)](const Glib::RefPtr<Glib::ObjectBase>& obj) {
                    auto info = std::dynamic_pointer_cast<Gio::FileInfo>(obj);
                    if (!info) return false;
                    FileInfo wrapped(info);
                    return pred(wrapped);
                }));
    }

    /** Internal: returns the underlying Gio::ListModel refptr. */
    Glib::RefPtr<Gio::ListModel> native_list_model() const noexcept;

    /** Internal: returns the underlying Gtk::FilterListModel refptr. */
    const Glib::RefPtr<Gtk::FilterListModel>& native() const noexcept { return m_model; }

private:
    explicit FilterListModel(Glib::RefPtr<Gtk::FilterListModel> m) : m_model(std::move(m)) {}
    static FilterListModel create_for_file_info_native(
        Glib::RefPtr<Gio::ListModel> source,
        sigc::slot<bool(const Glib::RefPtr<Glib::ObjectBase>&)> predicate);
    Glib::RefPtr<Gtk::FilterListModel> m_model;
};

/**
 * FileInfoSorter - custom Gtk::Sorter that compares two Gio::FileInfo objects.
 * The comparator is a template callable int(FileInfo&, FileInfo&) that
 * returns a negative/zero/positive value (< 0 : a before b; > 0 : a after b;
 * == 0 : equal). Used by the explorer to sort tree rows with directories
 * first and then alphabetical case insensitive.
 */
class FileInfoSorter {
public:
    /** Create a sorter from a comparator: int(FileInfo&, FileInfo&). */
    template <typename Comparator>
    static FileInfoSorter create(Comparator&& comparator) {
        return create_native(
            sigc::slot<int(const Glib::RefPtr<Gio::FileInfo>&,
                           const Glib::RefPtr<Gio::FileInfo>&)>(
                [cmp = std::forward<Comparator>(comparator)](
                    const Glib::RefPtr<Gio::FileInfo>& a,
                    const Glib::RefPtr<Gio::FileInfo>& b) -> int {
                    if (!a || !b) return 0;
                    FileInfo wa(a);
                    FileInfo wb(b);
                    return cmp(wa, wb);
                }));
    }

    /** Internal: returns the underlying Gtk::Sorter refptr. */
    const Glib::RefPtr<Gtk::Sorter>& native() const noexcept { return m_sorter; }

private:
    explicit FileInfoSorter(Glib::RefPtr<Gtk::Sorter> s) : m_sorter(std::move(s)) {}
    static FileInfoSorter create_native(
        sigc::slot<int(const Glib::RefPtr<Gio::FileInfo>&,
                       const Glib::RefPtr<Gio::FileInfo>&)> comparator);
    Glib::RefPtr<Gtk::Sorter> m_sorter;
};

/**
 * SortListModel - wraps another ListModel and presents its items ordered
 * according to a Sorter. Used downstream of FilterListModel to sort the
 * remaining directory entries (dirs before files, alphabetical within each
 * group). Changes in the source model are re-sorted automatically.
 */
class SortListModel {
public:
    /** Create a SortListModel wrapping the given filtered source with a sorter. */
    static SortListModel create(FilterListModel& source, FileInfoSorter& sorter);

    /** Internal: returns the underlying Gio::ListModel refptr. */
    Glib::RefPtr<Gio::ListModel> native_list_model() const noexcept;

    /** Internal: returns the underlying Gtk::SortListModel refptr. */
    const Glib::RefPtr<Gtk::SortListModel>& native() const noexcept { return m_model; }

private:
    explicit SortListModel(Glib::RefPtr<Gtk::SortListModel> m) : m_model(std::move(m)) {}
    Glib::RefPtr<Gtk::SortListModel> m_model;
};

/**
 * TreeListRow - one row in a TreeListModel. Wraps Gtk::TreeListRow and exposes
 * the row's underlying item (a FileInfo when used with DirectoryList) plus the
 * expand/collapse state.
 */
class TreeListRow {
public:
    explicit TreeListRow(Glib::RefPtr<Gtk::TreeListRow> row) : m_row(std::move(row)) {}

    /** Returns the item this row represents, as a FileInfo (or null if not). */
    FileInfo get_file_info() const;

    /** Get/set the expanded state. */
    bool get_expanded() const noexcept;
    void set_expanded(bool expanded);

    /** Returns the depth of this row in the tree (root rows have depth 0). */
    unsigned int get_depth() const noexcept;

    /** Returns true if this row may have child rows (i.e., it is a directory). */
    bool is_expandable() const noexcept;

    /** Returns the children list model (or null if not yet expanded). */
    Glib::RefPtr<Gio::ListModel> get_children_native() const noexcept;

    /** Internal: returns the underlying Gtk::TreeListRow refptr. */
    const Glib::RefPtr<Gtk::TreeListRow>& native() const noexcept { return m_row; }

private:
    Glib::RefPtr<Gtk::TreeListRow> m_row;
};

/**
 * TreeListModel - turns a flat ListModel into a tree by invoking a child-creator
 * callback for each row. The callback receives a FileInfo and returns either an
 * empty model (leaf) or another DirectoryList/FilterListModel/SortListModel
 * (subtree).
 */
class TreeListModel {
public:
    /** Create a TreeListModel from a filtered root model. */
    template <typename ChildCreator>
    static TreeListModel create(
        const FilterListModel& root,
        bool passthrough,
        bool autoexpand,
        ChildCreator&& child_creator)
    {
        return create_native(
            root.native_list_model(),
            passthrough,
            autoexpand,
            wrap_child_creator(std::forward<ChildCreator>(child_creator)));
    }

    /** Create a TreeListModel from a sorted root model (dirs first, alphabetical). */
    template <typename ChildCreator>
    static TreeListModel create(
        const SortListModel& root,
        bool passthrough,
        bool autoexpand,
        ChildCreator&& child_creator)
    {
        return create_native(
            root.native_list_model(),
            passthrough,
            autoexpand,
            wrap_child_creator(std::forward<ChildCreator>(child_creator)));
    }

    /**
     * Create a TreeListModel from any raw Gio::ListModel (e.g. a Gio::ListStore
     * built synchronously by the caller). Use this when the source model is
     * already filtered and sorted and the caller does not want to go through
     * the FilterListModel/SortListModel adapter types.
     */
    template <typename ChildCreator>
    static TreeListModel create(
        Glib::RefPtr<Gio::ListModel> root,
        bool passthrough,
        bool autoexpand,
        ChildCreator&& child_creator)
    {
        return create_native(
            std::move(root),
            passthrough,
            autoexpand,
            wrap_child_creator(std::forward<ChildCreator>(child_creator)));
    }

private:
    template <typename ChildCreator>
    static sigc::slot<Glib::RefPtr<Gio::ListModel>(const Glib::RefPtr<Glib::ObjectBase>&)>
    wrap_child_creator(ChildCreator&& child_creator) {
        return sigc::slot<Glib::RefPtr<Gio::ListModel>(const Glib::RefPtr<Glib::ObjectBase>&)>(
            [cc = std::forward<ChildCreator>(child_creator)](const Glib::RefPtr<Glib::ObjectBase>& obj)
                -> Glib::RefPtr<Gio::ListModel>
            {
                auto info = std::dynamic_pointer_cast<Gio::FileInfo>(obj);
                if (!info) return {};
                FileInfo wrapped(info);
                return cc(wrapped);
            });
    }

public:

    /** Internal: returns the underlying Gio::ListModel refptr (for SelectionModel/etc). */
    Glib::RefPtr<Gio::ListModel> native_list_model() const noexcept;

    /** Internal: returns the underlying Gtk::TreeListModel refptr. */
    const Glib::RefPtr<Gtk::TreeListModel>& native() const noexcept { return m_model; }

private:
    explicit TreeListModel(Glib::RefPtr<Gtk::TreeListModel> m) : m_model(std::move(m)) {}
    static TreeListModel create_native(
        Glib::RefPtr<Gio::ListModel> root,
        bool passthrough,
        bool autoexpand,
        sigc::slot<Glib::RefPtr<Gio::ListModel>(const Glib::RefPtr<Glib::ObjectBase>&)> child_creator);
    Glib::RefPtr<Gtk::TreeListModel> m_model;
};

/**
 * TreeExpander - widget that draws the indent level and expand arrow for a tree
 * row. Used inside ListView factory bind callbacks to wrap the row's content
 * widget with the proper tree-row decoration.
 */
class TreeExpander {
public:
    static TreeExpander create();

    void set_list_row(TreeListRow& row);

    template <typename W>
    void set_child(W& child) { set_child_native(child.native_widget()); }

    void set_margin_start(int m)  { detail::widget_set_margin_start(m_expander, m); }
    void set_margin_end(int m)    { detail::widget_set_margin_end(m_expander, m); }
    void set_margin_top(int m)    { detail::widget_set_margin_top(m_expander, m); }
    void set_margin_bottom(int m) { detail::widget_set_margin_bottom(m_expander, m); }
    void set_margin_all(int m)    { detail::widget_set_margin_all(m_expander, m); }
    void set_hexpand(bool e)      { detail::widget_set_hexpand(m_expander, e); }
    void set_vexpand(bool e)      { detail::widget_set_vexpand(m_expander, e); }
    void add_css_class(const std::string& c)    { detail::widget_add_css_class(m_expander, c); }
    void remove_css_class(const std::string& c) { detail::widget_remove_css_class(m_expander, c); }
    void set_visible(bool v)      { detail::widget_set_visible(m_expander, v); }
    bool is_visible() const       { return detail::widget_get_visible(m_expander); }
    void set_size_request(int w, int h)         { detail::widget_set_size_request(m_expander, w, h); }
    void set_tooltip_text(const std::string& t) { detail::widget_set_tooltip_text(m_expander, t); }
    void grab_focus()             { detail::widget_grab_focus(m_expander); }
    Gtk::Widget* native_widget() const noexcept { return m_expander; }
    Gtk::TreeExpander* native() const noexcept { return m_expander; }

private:
    explicit TreeExpander(Gtk::TreeExpander* e) : m_expander(e) {}
    void set_child_native(Gtk::Widget* w);
    Gtk::TreeExpander* m_expander;
};

}  // namespace ase::adp::gtk
