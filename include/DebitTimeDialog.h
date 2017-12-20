/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef DEBIT_TIME_DIALOG_H_
#define DEBIT_TIME_DIALOG_H_

#include "Session.h"
#include "Updateable.h"
#include "DebitTime.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>

using namespace Wt;

class DebitTimeDialog : public WDialog {
  public:
    DebitTimeDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<DebitTime> debitTime, Wt::Dbo::ptr<User> forUser);

    Updateable *owner_;
    Session &session_;
    Wt::Dbo::ptr<DebitTime> debitTime_;

    Wt::WText *errorMessage;
    Wt::Dbo::ptr<User> forUser_;
};

#endif //DEBIT_TIME_DIALOG_H_
