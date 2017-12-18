/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef CLOCK_VIEW_H_
#define CLOCK_VIEW_H_

#include "User.h"
#include "Session.h"
#include "Updateable.h"
#include "AbsenceDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTimer.h>

using namespace Wt;

class ClockView : public WContainerWidget {
  public:
    ClockView(Session &session);

    void update(bool fullUpdate = true);

  private:
    Session &session_;

    Wt::WTimer timer;

    Wt::WText *todayText;
};

#endif //CLOCK_VIEW_H_
