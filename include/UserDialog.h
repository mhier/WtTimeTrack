/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef USER_DIALOG_H_
#define USER_DIALOG_H_

#include "Session.h"
#include "Updateable.h"
#include "User.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>

using namespace Wt;

class UserDialog : public WDialog {
  public:
    UserDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<User> user);

    Session &session_;
    Updateable *owner_;

    Wt::Dbo::ptr<User> user_;
};

#endif //USER_DIALOG_H_
