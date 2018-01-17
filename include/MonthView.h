/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef MONTH_VIEW_H_
#define MONTH_VIEW_H_

#include "User.h"
#include "Session.h"
#include "Updateable.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class MonthView : public WContainerWidget, public Updateable {
  public:
    MonthView(Session &session, Wt::Dbo::ptr<User> forUser);

    Session &session_;

    void update() override;

    Wt::WText *weekSummaryTexts[6];
    Wt::WText *monthSummaryText;

};

#endif //MONTH_VIEW_H_
