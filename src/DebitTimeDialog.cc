/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "DebitTimeDialog.h"
#include "DebitTimeList.h"

#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WComboBox.h>
#include <Wt/WDoubleSpinBox.h>

DebitTimeDialog::DebitTimeDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<DebitTime> debitTime,
                                 Wt::Dbo::ptr<User> forUser)
: Wt::WDialog("Arbeitsstunden"), owner_(owner), session_(session), debitTime_(debitTime), forUser_(forUser)
{
    contents()->addStyleClass("form-group");

    bool createNew = false;
    if(debitTime_.get() == nullptr) {
      debitTime_ = std::make_unique<DebitTime>();
      debitTime_.modify()->validFrom = WDate::currentDate();
      for(size_t i=0; i<5; ++i) debitTime_.modify()->workHoursPerWeekday[i] = 8;
      for(size_t i=5; i<7; ++i) debitTime_.modify()->workHoursPerWeekday[i] = 0;
      createNew = true;
    }

    auto user = session_.user();
    if(user->role != UserRole::Admin) return;
    Dbo::Transaction transaction(session_.session_);

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());
    grid->setColumnResizable(0, false, Wt::WLength(10, Wt::LengthUnit::FontEx));
    grid->setColumnResizable(0, false, Wt::WLength(50, Wt::LengthUnit::FontEx));

    grid->addWidget(std::make_unique<Wt::WText>("Gültig ab: "), 0, 0);
    auto validFrom = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    validFrom->setDate(debitTime_->validFrom);

    std::array<Wt::WDoubleSpinBox*, 7> workHoursPerWeekday;
    for(size_t i=0; i<7; ++i) {
      grid->addWidget(std::make_unique<Wt::WText>(DebitTimeList::dayOfWeekNames[i]), 1+i, 0);
      workHoursPerWeekday[i] = grid->addWidget(std::make_unique<Wt::WDoubleSpinBox>(), 1+i, 1);
      workHoursPerWeekday[i]->setValue(debitTime_->workHoursPerWeekday[i]);
    }

    contents()->setWidth(Wt::WLength(65, Wt::LengthUnit::FontEx));

    errorMessage = grid->addWidget(std::make_unique<Wt::WText>(), 8, 1);
    errorMessage->hide();

    if(!createNew) {
      std::string text = "Achtung, die Arbeitszeit wird rückwirkend geändert. Ist dies nicht gewünscht, bitte den "
                         "Button 'Arbeitszeitänderung eintragen...' verwenden!";
      grid->addWidget(std::make_unique<Wt::WText>(text), 9, 1);
    }

    if(!createNew) {   // existing entry might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        debitTime_.remove();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    ok->clicked().connect(this, [=] {
      dbo::Transaction transaction(session_.session_);

      // check if edit is valid
      if(createNew || debitTime_->validFrom != validFrom->date()) {
        if(!forUser_->debitTimes.find().where("validFrom = ?").bind(validFrom->date()).resultList().empty()) {
          errorMessage->setText("Fehler: Es wurde bereits eine Arbeitszeit mit Gültigkeit ab genau dem angegebenen Datum eingetragen!");
          errorMessage->show();
          return;
        }
      }
      for(size_t i=0; i<7; ++i) {
        if(workHoursPerWeekday[i]->value() < 0 || workHoursPerWeekday[i]->value() > 24) {
          errorMessage->setText("Fehler: Arbeitszeit für " + DebitTimeList::dayOfWeekNames[i] + " nicht gültig!");
          errorMessage->show();
          return;
        }
      }

      if(!validFrom->date().isValid()) {
        errorMessage->setText("Fehler: Bitte Datum angeben!");
        errorMessage->show();
        return;
      }

      // apply modifications
      debitTime_.modify()->validFrom = validFrom->date();
      for(size_t i=0; i<7; ++i) debitTime_.modify()->workHoursPerWeekday[i] = workHoursPerWeekday[i]->value();
      if(createNew) forUser_.modify()->debitTimes.insert(debitTime_);
      owner_->update();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [&] {hide();} );

}

