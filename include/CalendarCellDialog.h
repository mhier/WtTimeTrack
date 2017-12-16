/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef CALENDAR_CELL_DIALOG_H_
#define CALENDAR_CELL_DIALOG_H_

#include <Wt/WDialog.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

#include "CalendarCell.h"
#include "EntryDialog.h"
#include "AbsenceDialog.h"
#include "Updateable.h"

using namespace Wt;

class CalendarCellDialog : public WDialog, public Updateable {
  public:
    CalendarCellDialog(CalendarCell* cell);

    void update() override;

    CalendarCell* cell_;

    std::unique_ptr<EntryDialog> entryDialog_;
    std::unique_ptr<AbsenceDialog> absenceDialog_;
};

#endif //CALENDAR_CELL_DIALOG_H_
