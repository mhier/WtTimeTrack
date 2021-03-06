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
#include "PlannerCalendar.h"
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
    resize(100, 80);

    clicked().connect(this, &CalendarCell::showCellDialog);
}

void CalendarCell::update(const WDate& date) {
    date_ = date;

    clear();

    dbo::Transaction transaction(session_.session_);
    auto user = owner_->view_.forUser_;

    // month summary: only once per month
    if( owner_->rowCounter == 0 && date.dayOfWeek() == 1 ) {
      WDate firstMonthDay = WDate(owner_->currentYear(), owner_->currentMonth(), 1);
      int monthBalance = user->getBalanceForRange( firstMonthDay, firstMonthDay.addMonths(1).addDays(-1) );
      int transfer = user->getBalanceUntil( firstMonthDay.addDays(-1) );
      std::string text = "Dieser Monat: "+secondsToString(monthBalance)+"<br/>";
      text += "Übertrag vom Vormonat: "+secondsToString(transfer);
      owner_->view_.monthSummaryText->setText(text);
    }

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

    auto credit = user->getCreditForRange(date, date);
    addWidget(std::make_unique<WText>( "Ist: " + secondsToString(credit) + "<br/>" ));

    auto absence = user->checkAbsence(date);
    if(absence->reason != Absence::Reason::NotAbsent) {
      addWidget(std::make_unique<WText>( Absence::ReasonToString(absence->reason) ));
    }
    else {
      auto debit = user->getDebitForRange(date, date);
      addWidget(std::make_unique<WText>( "Soll: " + secondsToString(debit) ));
    }

    if(date.dayOfWeek() == 7) {  // just once per week
      int weekBalance = user->getBalanceForRange( date.addDays(-6), date );
      int transfer = user->getBalanceUntil( date );
      std::string text = "Woche: "+secondsToString(weekBalance)+"<br/>";
      text += "Übertrag: "+secondsToString(transfer);
      owner_->view_.weekSummaryTexts[owner_->rowCounter]->setText(text);

      owner_->rowCounter++;
    }
}

void CalendarCell::showCellDialog() {
    dialog_ = std::make_unique<CalendarCellDialog>(this, owner_->view_.forUser_);
    dialog_->show();
}
