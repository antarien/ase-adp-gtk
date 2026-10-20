#pragma once

/**
 * ASE GTK ADAPTER - COCKPIT CELLS
 *
 * @file        cockpit.hpp
 * @brief       The building blocks of the dense full-width settings pages of the native clients
 * @description ase-client-explorer and ase-client-viewer both lay out their preferences as
 *              cockpit pages: uppercase section strips, flush cells that stack a label above
 *              one control, and rows that share the width evenly among their cells. The
 *              widgets carry the .ase-cfg-* CSS classes each client's theme.cpp defines; the
 *              spacing comes from ase::theme (design_tokens.hpp). Both clients carried these
 *              builders for themselves until they were drawn here.
 *
 * @module      ase-adp-gtk
 * @layer       5 (Adapter)
 * @category    render/ui/widget
 * @design      DSGN_021
 * @usecase     usecase:101134 - lay out settings cells
 * @created     2026-10-19
 * @modified    2026-10-19
 * @version     00.00.01.00001 [seed]
 *
 * ADAPTER COMPLIANCE
 *
 * [ ] English only - identifiers, comments, messages
 * [ ] META fields complete, @layer 5 (Section 0)
 * [ ] Layer dependencies correct (Section 11)
 * [ ] Headings are bare capital lines, cuts are empty lines (Section 0.1)
 * [ ] Form defined in INST_ASE_LINT.md Section 22
 */

#include <ase/containers/vector.hpp>

#include <initializer_list>

typedef struct _GtkWidget GtkWidget;

namespace ase::adp::adw::cockpit {

/** An uppercase tinted section title (.ase-cfg-section-title) spanning the page width. */
GtkWidget* make_section_strip(const char* title);

/** A flush cell (.ase-cfg-cell) that stacks an uppercase label above `control`. */
GtkWidget* make_cell(const char* label_text, GtkWidget* control);

/** A switch set to `initial`, centred in its cell. */
GtkWidget* make_toggle(bool initial);

/** A drop-down over `choices` with `selected` chosen, centred in its cell. */
GtkWidget* make_dropdown(const ase::containers::Vector<const char*>& choices, unsigned selected);

/** A horizontal row of cells that share the row evenly. */
GtkWidget* make_cell_row(std::initializer_list<GtkWidget*> cells);

}  // namespace ase::adp::adw::cockpit
