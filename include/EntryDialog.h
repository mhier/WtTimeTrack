/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef ENTRY_DIALOG_H_
#define ENTRY_DIALOG_H_

#include "Session.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>

using namespace Wt;

class CalendarCellDialog;

class EntryDialog : public WDialog {
  public:
    EntryDialog(CalendarCellDialog *owner, Session &session, Wt::Dbo::ptr<CreditTime> entry);

    Session &session_;
    CalendarCellDialog *owner_;
    Wt::Dbo::ptr<CreditTime> entry_;

    Wt::WText *errorMessage;
};

#endif //ENTRY_DIALOG_H_
