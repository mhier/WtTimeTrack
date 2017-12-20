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

    Dbo::Transaction transaction(session_.session_);
    auto user = session_.user();
    if(user->role != UserRole::Admin) return;

    bool createNew = false;
    if(holiday_.get() == nullptr) {
      createNew = true;
      holiday_ = Wt::Dbo::ptr<Holiday>(std::make_unique<Holiday>());
    }

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());

    grid->addWidget(std::make_unique<Wt::WText>("Erster Tag: "), 0, 0);
    auto de1 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    de1->setDate(holiday_->first);

    grid->addWidget(std::make_unique<Wt::WText>("Letzter Tag: "), 1, 0);
    auto de2 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 1, 1);
    de2->setDate(holiday_->last);

    contents()->setWidth(600);

    if(!createNew) {   // existing absence might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        holiday_.remove();
        transaction.commit();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    ok->clicked().connect(this, [=] {
      dbo::Transaction transaction(session_.session_);
      holiday_.modify()->first = de1->date();
      holiday_.modify()->last = de2->date();
      if(createNew) {   // create new on ok
        session_.session_.add(holiday_);
      }
      transaction.commit();
      owner_->update();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [=] {hide();} );

    transaction.commit();

}

