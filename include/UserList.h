/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef USER_LIST_H_
#define USER_LIST_H_

#include "User.h"
#include "Session.h"
#include "Updateable.h"
#include "UserDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class UserList : public WContainerWidget, public Updateable {
  public:
    UserList(Session &session);

    void update() override;

  private:
    Session &session_;
    std::unique_ptr<UserDialog> userDialog_;
};

#endif //USER_LIST_H_
