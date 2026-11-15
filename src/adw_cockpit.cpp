/**
 * ASE GTK ADAPTER - COCKPIT CELLS
 *
 * @file        adw_cockpit.cpp
 * @brief       Implementation for adw/cockpit.hpp
 * @description Raw GTK4 widgets with the .ase-cfg-* CSS classes; the look is the theme of
 *              the calling client, the spacing is ase::theme from design_tokens.hpp.
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

#include <ase/adp/adw/cockpit.hpp>

#include "design_tokens.hpp"

#include <gtk/gtk.h>

namespace ase::adp::adw::cockpit {

namespace t = ase::theme;

GtkWidget* make_section_strip(const char* title) {
    GtkWidget* label = gtk_label_new(title);
    gtk_widget_add_css_class(label, "ase-cfg-section-title");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    return label;
}

GtkWidget* make_cell(const char* label_text, GtkWidget* control) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, t::space_xs);
    gtk_widget_add_css_class(box, "ase-cfg-cell");
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, FALSE);

    GtkWidget* label = gtk_label_new(label_text);
    gtk_widget_add_css_class(label, "ase-cfg-cell-label");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget* value_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, t::space_sm);
    gtk_widget_add_css_class(value_row, "ase-cfg-cell-value");
    gtk_widget_set_hexpand(value_row, TRUE);
    gtk_box_append(GTK_BOX(value_row), control);
    gtk_widget_set_hexpand(control, TRUE);
    gtk_widget_set_halign(control, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), value_row);

    return box;
}

GtkWidget* make_toggle(bool initial) {
    GtkWidget* sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(sw), initial ? TRUE : FALSE);
    gtk_widget_set_valign(sw, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(sw, GTK_ALIGN_START);
    return sw;
}

GtkWidget* make_dropdown(const ase::containers::Vector<const char*>& choices, unsigned selected) {
    // gtk_drop_down_new_from_strings reads up to a null entry.
    ase::containers::Vector<const char*> with_null(choices);
    with_null.push_back(nullptr);
    GtkWidget* dd = gtk_drop_down_new_from_strings(with_null.data());
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), selected);
    gtk_widget_set_valign(dd, GTK_ALIGN_CENTER);
    return dd;
}

GtkWidget* make_cell_row(std::initializer_list<GtkWidget*> cells) {
    GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(row, TRUE);
    for (GtkWidget* cell : cells) {
        gtk_box_append(GTK_BOX(row), cell);
    }
    return row;
}

}  // namespace ase::adp::adw::cockpit
