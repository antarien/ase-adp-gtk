#pragma once

/**
 * @file        adw.hpp
 * @brief       Umbrella header for the ase::adp::adw wrappers
 * @description Pulls in every Adwaita wrapper so client code can include this
 *              single header to access the full ASE-native libadwaita surface.
 *              Individual headers may also be included directly for finer
 *              compile-dependency control.
 *
 * @module      ase-adp-gtk
 * @layer       adapter
 */

#include <ase/adp/gtk/_fwd.hpp>
#include <ase/adp/adw/window.hpp>
#include <ase/adp/adw/preferences.hpp>
