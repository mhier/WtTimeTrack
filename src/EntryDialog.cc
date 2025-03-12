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
    contents()->setHeight(Wt::WLength(7, Wt::LengthUnit::Pica));

    dbo::Transaction transaction(session_.session_);
    auto user = owner_->forUser_;

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

    errorMessage = grid->addWidget(std::make_unique<Wt::WText>(), 2, 1);
    errorMessage->hide();

    contents()->setWidth(600);

    if(!createNew) {   // existing entries can be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);

        // prevent edit after year is closed
        bool editAfterClose = false;
        if(session_.isYearClosed(entry->start.date().year())) editAfterClose = true;
        if(session_.isYearClosed(entry->stop.date().year())) editAfterClose = true;
        if(editAfterClose) {
          errorMessage->setText("Fehler: Buchungen in geschlossenen Jahren können nicht gelöscht werden!");
          errorMessage->show();
          return;
        }

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
    ok->clicked().connect(this, [=] {
      dbo::Transaction transaction(session_.session_);

      // prevent edit after year is closed
      bool editAfterClose = false;
      if(session_.isYearClosed(de1->date().year())) editAfterClose = true;
      if(de2 && session_.isYearClosed(de2->date().year())) editAfterClose = true;
      if(!createNew) {
        if(session_.isYearClosed(entry->start.date().year())) editAfterClose = true;
        if(session_.isYearClosed(entry->stop.date().year())) editAfterClose = true;
      }
      if(editAfterClose) {
        errorMessage->setText("Fehler: Buchungen in geschlossenen Jahren können nicht bearbeitet oder hinzugefügt werden!");
        errorMessage->show();
        return;
      }

      // check if edit is valid
      if(!de1->date().isValid() || !te1->time().isValid()) {
        errorMessage->setText("Fehler: Das erste Buchungsdatum muss vollständig angegeben werden!");
        errorMessage->show();
        return;
      }
      Wt::WDateTime dt1 = WLocalDateTime(de1->date(), te1->time()).toUTC();
      Wt::WDateTime dt2;
      if(entry->hasClockedOut) {
        if(!de2->date().isValid() || !te2->time().isValid()) {
          errorMessage->setText("Fehler: Der letzte Buchungsdatum muss vollständig angegeben werden!");
          errorMessage->show();
          return;
        }
        dt2 = WLocalDateTime(de2->date(), te2->time()).toUTC();

        if(dt1.secsTo(dt2) < 0) {
          errorMessage->setText("Fehler: Das Ende der Buchung liegt vor dem Anfang!");
          errorMessage->show();
          return;
        }
      }

      { // make sure the temporary Dbo::collections get destroyed before owner_->update() is called
        auto temp1 = user->creditTimes.find().where("start <= ?").bind(dt1)
                                            .where("stop > ?").bind(dt1).resultList();
        if(!temp1.empty() && temp1.front().id() != entry.id()) {
          errorMessage->setText("Fehler: Die Buchung überlappt mit einer anderen Buchung!");
          errorMessage->show();
          return;
        }
        auto temp2 = user->creditTimes.find().where("start >= ?").bind(dt1)
                                            .where("start < ?").bind(dt2).resultList();
        if(!temp2.empty() && temp2.front().id() != entry.id()) {
          errorMessage->setText("Fehler: Die Buchung überlappt mit einer anderen Buchung!");
          errorMessage->show();
          return;
        }
      }

      entry.modify()->start = dt1;
      if(entry->hasClockedOut) entry.modify()->stop = dt2;
      if(createNew) owner_->forUser_.modify()->creditTimes.insert(entry);
      owner_->update();
      owner_->cell_->owner_->browseToPreviousMonth();   // update all calendar cells
      owner_->cell_->owner_->browseToNextMonth();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [&] {hide();} );

}
