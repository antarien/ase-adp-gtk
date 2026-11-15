#pragma once

/**
 * ASE GTK ADAPTER - FOLDER PICKER
 *
 * @file        folder_picker.hpp
 * @brief       Modal folder picker built from AdwWindow and GtkListBox, without a portal
 * @description The one folder picker of the native clients. ase-client-explorer and
 *              ase-client-viewer both open it from their settings to choose a root folder;
 *              each carried its own copy until both were drawn here. It replaces
 *              GtkFileDialog, whose GtkPathBar fallback fires "gtk_box_remove: GTK_IS_BOX (box)
 *              failed" cascades on Hyprland sessions where no FileChooser portal backend is
 *              eligible.
 *
 *              Supports: navigate into a subdirectory by clicking, jump up via the parent
 *              button, type an absolute path into the entry, confirm via Open, cancel via
 *              Cancel or Escape.
 *
 *              The callback is a function pointer with user_data, not a function object: GTK
 *              calls back only in this form, and both callers already hold their state behind
 *              a user_data pointer that the callback unpacks as its first line.
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

#include <string>

typedef struct _GtkWindow GtkWindow;

namespace ase::adp::adw::folder_picker {

/**
 * Called with the absolute path the user accepted. `user_data` is handed back
 * unchanged from the show() call, exactly as a GTK signal handler receives it.
 */
using SelectedFn = void (*)(const std::string& path, void* user_data);

/**
 * Present a modal folder picker as a child of `parent`. Starts at
 * `start_path` (or HOME if empty/non-existent). Calls `on_selected` with
 * the absolute path when the user accepts; not called on cancel.
 *
 * `user_data` must outlive the picker window. The picker neither owns nor
 * frees it.
 */
void show(GtkWindow* parent,
          const std::string& start_path,
          SelectedFn on_selected,
          void* user_data);

}  // namespace ase::adp::adw::folder_picker
