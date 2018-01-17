/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "EntryDialog.h"
#include "CalendarCellDialog.h"
#include "PlannerCalendar.h"

#include <Wt/WPushButton.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WTable.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WDateEdit.h>
#include <Wt/WGridLayout.h>

#include <Wt/Dbo/WtSqlTraits.h>

EntryDialog::EntryDialog(CalendarCellDialog *owner, Session &session, Wt::Dbo::ptr<CreditTime> entry)
: WDialog("Buchung"), session_(session), owner_(owner), entry_(entry)
{
    contents()->addStyleClass("form-group");

    dbo::Transaction transaction(session_.session_);
    auto user = session_.user();

    bool createNew = false;
    if(entry.get() == nullptr) {
      entry = std::make_unique<CreditTime>();
      entry.modify()->start = WDateTime(owner->cell_->date(), WTime(9,0));
      entry.modify()->stop = WDateTime(owner->cell_->date(), WTime(17,0));
      entry.modify()->hasClockedOut = true;
      createNew = true;
    }

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());

    grid->addWidget(std::make_unique<Wt::WText>("Eingang: "), 0, 0);
    auto de1 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    de1->setDate(entry->start.toLocalTime().date());
    auto te1 = grid->addWidget(std::make_unique<Wt::WTimeEdit>(), 0, 2);
    te1->setTime(entry->start.toLocalTime().time());

    grid->addWidget(std::make_unique<Wt::WText>("Ausgang: "), 1, 0);
    Wt::WDateEdit *de2{nullptr};
    Wt::WTimeEdit *te2{nullptr};
    if(entry->hasClockedOut) {
      de2 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 1, 1);
      de2->setDate(entry->stop.toLocalTime().date());
      te2 = grid->addWidget(std::make_unique<Wt::WTimeEdit>(), 1, 2);
      te2->setTime(entry->stop.toLocalTime().time());
    }
    else {
      grid->addWidget(std::make_unique<Wt::WText>("(nicht ausgestempelt)"), 1, 1);
    }

    contents()->setWidth(600);

    if(!createNew) {   // existing entry might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        auto entries = user->creditTimes.find().where("id = ?").bind(entry.id()).resultList();
        entries.front().remove();
        owner_->update();
        owner_->cell_->owner_->browseToPreviousMonth();   // update all calendar cells
        owner_->cell_->owner_->browseToNextMonth();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    if(createNew) {   // create new on ok
      ok->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        entry.modify()->start = WLocalDateTime(de1->date(), te1->time()).toUTC();
        if(entry->hasClockedOut) entry.modify()->stop = WLocalDateTime(de2->date(), te2->time()).toUTC();
        session_.user().modify()->creditTimes.insert(entry);
        owner_->update();
        owner_->cell_->owner_->browseToPreviousMonth();   // update all calendar cells
        owner_->cell_->owner_->browseToNextMonth();
        hide();
      } );
    }
    else {
      ok->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        entry.modify()->start = WLocalDateTime(de1->date(), te1->time()).toUTC();
        if(entry->hasClockedOut) entry.modify()->stop = WLocalDateTime(de2->date(), te2->time()).toUTC();
        owner_->update();
        owner_->cell_->owner_->browseToPreviousMonth();   // update all calendar cells
        owner_->cell_->owner_->browseToNextMonth();
        hide();
      } );
    }

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [&] {hide();} );

}
