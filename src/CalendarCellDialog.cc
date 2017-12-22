/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "CalendarCellDialog.h"
#include "PlannerCalendar.h"

#include <Wt/WTemplate.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WTable.h>

#include <Wt/Dbo/WtSqlTraits.h>

CalendarCellDialog::CalendarCellDialog(CalendarCell* cell)
: WDialog(cell->date().toString("ddd, d MMM yyyy")), cell_(cell)
{
    update();
}

void CalendarCellDialog::update() {
    contents()->clear();
    footer()->clear();
    contents()->addStyleClass("form-group");

    dbo::Transaction transaction(cell_->session_.session_);
    auto user = cell_->session_.user();

    auto absence = user->checkAbsence(cell_->date());
    if(absence->reason != Absence::Reason::NotAbsent) {
      std::string text = Absence::ReasonToString(absence->reason);
      text += ": ";
      text += absence->first.toString("yyyy-MM-dd").toUTF8();
      if(absence->first != absence->last) {
        text += " bis ";
        text += absence->last.toString("yyyy-MM-dd").toUTF8();
      }
      contents()->addWidget(std::make_unique<WText>( text ));
    }

    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Eingang"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Ausgang"));
    table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Stunden"));

    auto creditTimes = user->creditTimesInRange(cell_->date(), cell_->date().addDays(1));
    int row = 0;
    for(auto creditTime : creditTimes) {
      row++;

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(creditTime->start.toString("HH:mm")));
      if(creditTime->hasClockedOut) {
        table->elementAt(row, 2)->addWidget(std::make_unique<WText>(creditTime->stop.toString("HH:mm")));
      }
      else {
        table->elementAt(row, 2)->addWidget(std::make_unique<WText>("--:--"));
      }

      table->elementAt(row, 3)->addWidget(std::make_unique<WText>( CreditTimePeriod(creditTime).getAsString() ));

      for(int i=0; i<4; ++i) {
        table->elementAt(row,i)->clicked().connect(this, [=] {
          entryDialog_ = std::make_unique<EntryDialog>(this, cell_->session_, creditTime);
          entryDialog_->show();
        });
      }
    }

    contents()->addWidget(std::move(table));

    Wt::WPushButton *newEntry = footer()->addWidget(std::make_unique<Wt::WPushButton>("Neue Buchung..."));
    newEntry->clicked().connect(this, [=] {
        entryDialog_ = std::make_unique<EntryDialog>(this, cell_->session_, nullptr);
        entryDialog_->show();
    } );

    std::string absenceButtonTitle;
    if(absence->reason != Absence::Reason::NotAbsent) {
      absenceButtonTitle = "Abwesenheit bearbeiten...";
      Wt::WPushButton *newAbsence = footer()->addWidget(std::make_unique<Wt::WPushButton>(absenceButtonTitle));
      newAbsence->clicked().connect(this, [=] {
        absenceDialog_ = std::make_unique<AbsenceDialog>(this, cell_->session_, absence);
        absenceDialog_->show();
      } );
    }
    else {
      absenceButtonTitle = "Abwesenheit melden...";
      Wt::WPushButton *newAbsence = footer()->addWidget(std::make_unique<Wt::WPushButton>(absenceButtonTitle));
      newAbsence->clicked().connect(this, [=] {
        absenceDialog_ = std::make_unique<AbsenceDialog>(this, cell_->session_, nullptr, cell_->date());
        absenceDialog_->show();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Schließen"));
    ok->setDefault(true);
    ok->clicked().connect(this, [=] {
      this->hide();
    });

    // update all calendar cells - this is required if update() is called from child dialogs
    cell_->owner_->browseToPreviousMonth();
    cell_->owner_->browseToNextMonth();
}
