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

#include "PlannerCalendar.h"
#include "CalendarCellDialog.h"

AbsenceDialog::AbsenceDialog(Updateable *owner, Session &session, WDate suggestedStartDate)
: owner_(owner), Wt::WDialog("Abwesenheit"), session_(session)
{
    contents()->addStyleClass("form-group");

    auto user = session_.user();
    Dbo::Transaction transaction(session_.session_);

    auto absence = user->checkAbsence(suggestedStartDate);
    bool createNew = false;
    if(absence->reason == Absence::Reason::NotAbsent) {     // create new absence (otherwise: edit existing)
      createNew = true;
      absence.modify()->reason = Absence::Reason::Holiday;
      absence.modify()->first = suggestedStartDate;
      absence.modify()->last = suggestedStartDate;
    }

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());

    grid->addWidget(std::make_unique<Wt::WText>("Erster Abwesenheitstag: "), 0, 0);
    auto de1 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 0, 1);
    de1->setDate(absence.modify()->first);

    grid->addWidget(std::make_unique<Wt::WText>("Letzter Abwesenheitstag: "), 1, 0);
    auto de2 = grid->addWidget(std::make_unique<Wt::WDateEdit>(), 1, 1);
    de2->setDate(absence.modify()->last);

    grid->addWidget(std::make_unique<Wt::WText>("Grund: "), 2, 0);
    auto cb = grid->addWidget(std::make_unique<Wt::WComboBox>(), 2, 1);
    cb->addItem(Absence::ReasonToString(Absence::Reason::Holiday));
    cb->addItem(Absence::ReasonToString(Absence::Reason::SpecialLeave));
    cb->addItem(Absence::ReasonToString(Absence::Reason::Illness));
    cb->setCurrentIndex(static_cast<int>(absence.modify()->reason)-1);

    contents()->setWidth(600);

    if(!createNew) {   // existing absence might be deleted
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        auto absence = user->checkAbsence(suggestedStartDate);
        absence.remove();
        transaction.commit();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    if(createNew) {   // create new on ok
      ok->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        Wt::Dbo::ptr<Absence> absence(std::make_unique<Absence>());
        absence.modify()->first = de1->date();
        absence.modify()->last = de2->date();
        absence.modify()->reason = Absence::StringToReason(cb->currentText());
        session_.user().modify()->absences.insert(absence);
        transaction.commit();
        owner_->update();
        hide();
      } );
    }
    else {
      ok->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        auto absence = user->checkAbsence(suggestedStartDate);
        absence.modify()->first = de1->date();
        absence.modify()->last = de2->date();
        absence.modify()->reason = Absence::StringToReason(cb->currentText());
        transaction.commit();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [&] {hide();} );

    transaction.commit();

}

