/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef ABSENCE_LIST_H_
#define ABSENCE_LIST_H_

#include "User.h"
#include "Session.h"
#include "Updateable.h"
#include "AbsenceDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class AbsenceList : public WContainerWidget, public Updateable {
  public:
    AbsenceList(Session &session,  Wt::Dbo::ptr<User> forUser);

    void update() override;

  private:
    Session &session_;
    std::unique_ptr<AbsenceDialog> absenceDialog_;
    int year{0};
};

#endif //ABSENCE_LIST_H_
