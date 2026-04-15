#pragma once

/**
 * @file        gtk.hpp
 * @brief       Umbrella header for the ase::adp::gtk wrappers
 * @description Pulls in every wrapper class so client code can include this
 *              single header to access the full ASE-native gtkmm surface.
 *              Individual headers may also be included directly for finer
 *              compile-dependency control.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>
#include <ase/adp/gtk/widget.hpp>
#include <ase/adp/gtk/application.hpp>
#include <ase/adp/gtk/list_view.hpp>
#include <ase/adp/gtk/tree.hpp>
#include <ase/adp/gtk/menu.hpp>
#include <ase/adp/gtk/gesture.hpp>
#include <ase/adp/gtk/io.hpp>
#include <ase/adp/gtk/style.hpp>
