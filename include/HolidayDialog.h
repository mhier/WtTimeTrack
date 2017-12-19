/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef HOLIDAY_DIALOG_H_
#define HOLIDAY_DIALOG_H_

#include "Session.h"
#include "Updateable.h"
#include "Holiday.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>

using namespace Wt;

class HolidayDialog : public WDialog {
  public:
    HolidayDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<Holiday> holiday);

    Session &session_;
    Updateable *owner_;

    Wt::Dbo::ptr<Holiday> holiday_;
};

#endif //HOLIDAY_DIALOG_H_
