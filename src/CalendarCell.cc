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
    auto user = session_.user();

    // compute row in calendar and store if this is the first cell
    bool firstCell = false;
    if(date.dayOfWeek() == 1) {
      // If this is a Monday and it belongs to the previous month -> first cell!
      if( (date.month() < owner_->currentMonth() && date.year() == owner_->currentYear()) ||
          (date.year()  < owner_->currentYear())                                              ) {
        owner_->rowCounter = 0;
        firstCell = true;
      }
      else {
        owner_->rowCounter++;
      }
    }

    // month summary: only once per month
    if(firstCell) {
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
    }
}

void CalendarCell::showCellDialog() {
    dialog_ = std::make_unique<CalendarCellDialog>(this);
    dialog_->show();
}
