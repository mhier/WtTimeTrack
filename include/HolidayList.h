/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef HOLIDAY_LIST_H_
#define HOLIDAY_LIST_H_

#include "Holiday.h"
#include "Session.h"
#include "Updateable.h"
#include "HolidayDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class HolidayList : public WContainerWidget, public Updateable {
  public:
    HolidayList(Session &session);

    void update() override;

  private:
    Session &session_;
    std::unique_ptr<HolidayDialog> holidayDialog_;

    int year{0};
};

#endif //HOLIDAY_LIST_H_
