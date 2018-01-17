/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "AbsenceDialog.h"

#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WComboBox.h>

AbsenceDialog::AbsenceDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<Absence> absence, Wt::WDate newDate)
: Wt::WDialog("Abwesenheit"), owner_(owner), session_(session), absence_(absence)
{
    contents()->addStyleClass("form-group");

    auto user = owner_->forUser_;
    Dbo::Transaction transaction(session_.session_);

    bool createNew = false;
    // create new absence (otherwise: edit existing)
    if(absence_.get() == nullptr) {
      createNew = true;
      absence_ = std::make_unique<Absence>();
      absence_.modify()->reason = Absence::Reason::Holiday;
      if(newDate.isValid()) {
        absence_.modify()->first = newDate;
        absence_.modify()->last = newDate;
      }
      else {
        absence_.modify()->first = WDate::currentDate();
        absence_.modify()->last = WDate::currentDate();
      }
    }

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());

    grid->addWidget(std::make_unique<Wt::WText>("Erster Abwesenheitstag: "), 0, 0);
    auto de1 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    de1->setDate(absence_.modify()->first);

    grid->addWidget(std::make_unique<Wt::WText>("Letzter Abwesenheitstag: "), 1, 0);
    auto de2 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 1, 1);
    de2->setDate(absence_.modify()->last);

    grid->addWidget(std::make_unique<Wt::WText>("Grund: "), 2, 0);
    auto cb = grid->addWidget(std::make_unique<Wt::WComboBox>(), 2, 1);
    cb->addItem(Absence::ReasonToString(Absence::Reason::Holiday));
    cb->addItem(Absence::ReasonToString(Absence::Reason::SpecialLeave));
    cb->addItem(Absence::ReasonToString(Absence::Reason::Illness));
    cb->addItem(Absence::ReasonToString(Absence::Reason::OfficialTrip));
    cb->setCurrentIndex(static_cast<int>(absence_.modify()->reason)-1);

    errorMessage = grid->addWidget(std::make_unique<Wt::WText>(), 3, 1);
    errorMessage->hide();

    contents()->setWidth(600);

    if(!createNew) {   // existing absence might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        absence_.remove();
        owner_->forUser_->invalidateCaches();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    ok->clicked().connect(this, [=] {
      dbo::Transaction transaction(session_.session_);

      // check if edit is valid
      if(!de1->date().isValid() || !de2->date().isValid()) {
        errorMessage->setText("Fehler: Erster und letzter Abwesenheitstag muss angegeben werden!");
        errorMessage->show();
        return;
      }

      if(de1->date() > de2->date()) {
        errorMessage->setText("Fehler: Das Ende der Abwesenheit liegt vor dem Anfang!");
        errorMessage->show();
        return;
      }

      { // make sure temporary query results are gone before calling owner_->update()
        auto temp1 = owner_->forUser_->absences.find().where("first <= ?").bind(de1->date())
                                          .where("last >= ?").bind(de1->date()).resultList();
        if(!temp1.empty() && temp1.front().id() != absence_.id()) {
          errorMessage->setText("Fehler: Die Abwesenheit überlappt mit einer anderen Abwesenheit!");
          errorMessage->show();
          return;
        }
        auto temp2 = owner_->forUser_->absences.find().where("first >= ?").bind(de1->date())
                                          .where("first <= ?").bind(de2->date()).resultList();
        if(!temp2.empty() && temp2.front().id() != absence_.id()) {
          errorMessage->setText("Fehler: Die Abwesenheit überlappt mit einer anderen Abwesenheit!");
          errorMessage->show();
          return;
        }
      }

      // apply modifications
      absence_.modify()->first = de1->date();
      absence_.modify()->last = de2->date();
      absence_.modify()->reason = Absence::StringToReason(cb->currentText());
      if(createNew) owner_->forUser_.modify()->absences.insert(absence_);
      owner_->forUser_->invalidateCaches();
      owner_->update();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [&] {hide();} );

}

