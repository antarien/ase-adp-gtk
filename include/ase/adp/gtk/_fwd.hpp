#pragma once

/**
 * @file        _fwd.hpp
 * @brief       Forward declarations for ase::adp::gtk wrappers
 * @description Lets header files reference sibling wrapper types without including
 *              full definitions (and without dragging gtkmm headers transitively).
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

namespace ase::adp::gtk {

// Core / lifecycle
class Application;
class ApplicationWindow;

// Containers
class Widget;
class Box;
class ScrolledWindow;

// Controls
class Button;
class Label;
class HeaderBar;
class SearchEntry;

// List / Tree
class ListView;
class ListItem;
class ListItemFactory;
class SingleSelection;
class TreeListModel;
class TreeListRow;
class TreeExpander;
class DirectoryList;
class FilterListModel;

// Menus
class PopoverMenu;
class Menu;
class ActionGroup;

// Gestures / events
class ClickGesture;
class KeyController;
class DragSource;

// File I/O
class File;
class FileMonitor;
class FileInfo;
class FileLauncher;

// Style
class CssProvider;

}  // namespace ase::adp::gtk

namespace ase::adp::adw {

class Window;
class ToolbarView;
class HeaderBar;
class ViewStack;
class ViewSwitcher;
class StyleManager;
class PreferencesPage;
class PreferencesGroup;
class SwitchRow;
class EntryRow;
class ComboRow;
class SpinRow;

}  // namespace ase::adp::adw
