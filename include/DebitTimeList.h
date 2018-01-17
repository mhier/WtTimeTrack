/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef DEBIT_TIME_LIST_H_
#define DEBIT_TIME_LIST_H_

#include <array>
#include <string>

#include "User.h"
#include "Session.h"
#include "Updateable.h"
#include "DebitTimeDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class DebitTimeList : public WContainerWidget, public Updateable {
  public:
    DebitTimeList(Session &session, Wt::Dbo::ptr<User> forUser);

    void update() override;

    static const std::array<std::string, 7> dayOfWeekNames;

    static std::string formatNumber(double hours);

  private:
    Session &session_;
    std::unique_ptr<DebitTimeDialog> debitTimeDialog_;
};

#endif //DEBIT_TIME_LIST_H_

