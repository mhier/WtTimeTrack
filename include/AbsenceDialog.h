/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef ABSENCE_DIALOG_H_
#define ABSENCE_DIALOG_H_

#include "Session.h"
#include "Updateable.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>

using namespace Wt;

class AbsenceDialog : public WDialog {
  public:
    AbsenceDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<Absence> absence);

    Session &session_;
    Updateable *owner_;

    Wt::Dbo::ptr<Absence> absence_;

    Wt::WText *errorMessage;

};

#endif //ABSENCE_DIALOG_H_
