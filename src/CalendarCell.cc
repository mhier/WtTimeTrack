/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "CalendarCell.h"
#include "CalendarCellDialog.h"
#include <Wt/WDialog.h>

#include <Wt/WDate.h>
#include <Wt/WText.h>

#include <string>
#include <cmath>

#include "utils.h"

CalendarCell::CalendarCell(Session &session)
  : WContainerWidget(),
    session_(session)
{
    resize(80, 80);

    clicked().connect(this, &CalendarCell::showCellDialog);
}

void CalendarCell::update(const WDate& date) {
    date_ = date;

    clear();

    dbo::Transaction transaction(session_.session_);
    auto user = session_.user();

    WString day;
    day += std::to_string(date.day());
    if (date.day() == 1) day += " " + WDate::longMonthName(date.month());
    auto header = std::make_unique<WText>(day);
    header->setStyleClass("cell-header");
    addWidget(std::move(header));

    if(date != WDate::currentDate()) {
      setStyleClass("cell");
    }
    else {
      setStyleClass("cell-today");
    }

    //auto period = CreditTimePeriod(user->creditTimesInRange(date, date.addDays(1)));
    //addWidget(std::make_unique<WText>( "Ist: " + period.getAsString() ));

    auto credit = user->getCreditForRange(date, date.addDays(1));
    addWidget(std::make_unique<WText>( "Ist: " + secondsToString(credit)));

    std::string debitTime = secondsToString(user->getDebitTimeForDate(date_) * 3600.);
    addWidget(std::make_unique<WText>( "Soll: " + debitTime ));

    auto absence = user->checkAbsence(date);
    if(absence->reason != Absence::Reason::NotAbsent) {
      addWidget(std::make_unique<WText>( Absence::ReasonToString(absence->reason) ));
    }

    if(date.dayOfWeek() == 1) {  // Monday: show transfer from last week

    }
    else if(date.dayOfWeek() == 7) {   // Sunday: show week balance
      double weekBalance = user->getBalanceForRange( date.addDays(-6), date );
      std::string sWeekBalance = "Woche: "+secondsToString(weekBalance * 3600.);
      addWidget(std::make_unique<WText>(sWeekBalance));
    }

    transaction.commit();
}

void CalendarCell::showCellDialog() {
    dialog_ = std::make_unique<CalendarCellDialog>(this);
    dialog_->show();
}
