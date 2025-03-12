/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "HolidayDialog.h"

#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WComboBox.h>

HolidayDialog::HolidayDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<Holiday> holiday)
: Wt::WDialog("Feiertag/Betriebsferien"), owner_(owner), session_(session), holiday_(holiday)
{
    contents()->addStyleClass("form-group");
    contents()->setHeight(Wt::WLength(7, Wt::LengthUnit::Pica));

    Dbo::Transaction transaction(session_.session_);
    auto user = session_.user();
    if(user->role != UserRole::Admin) return;

    bool createNew = false;
    if(holiday_.get() == nullptr) {
      createNew = true;
      holiday_ = Wt::Dbo::ptr<Holiday>(std::make_unique<Holiday>());
    }

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());
    grid->setColumnStretch(0,0);
    grid->setColumnStretch(1,1);

    grid->addWidget(std::make_unique<Wt::WText>("Erster Tag: "), 0, 0);
    auto de1 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    de1->setDate(holiday_->first);

    grid->addWidget(std::make_unique<Wt::WText>("Letzter Tag: "), 1, 0);
    auto de2 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 1, 1);
    de2->setDate(holiday_->last);

    errorMessage = grid->addWidget(std::make_unique<Wt::WText>(), 8, 1);
    errorMessage->hide();

    if(!createNew) {   // existing absence might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        holiday_.remove();
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
        errorMessage->setText("Fehler: Bitte ersten und letzten Tag angeben!");
        errorMessage->show();
        return;
      }

      // check if edit is valid
      if(de1->date() > de2->date()) {
        errorMessage->setText("Fehler: Der letzte Tag darf nicht vor dem ersten liegen!");
        errorMessage->show();
        return;
      }

      // update the holiday object
      holiday_.modify()->first = de1->date();
      holiday_.modify()->last = de2->date();
      if(createNew) {   // create new on ok
        session_.session_.add(holiday_);
      }
      owner_->update();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [=] {hide();} );

}

