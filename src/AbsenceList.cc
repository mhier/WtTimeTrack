/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "AbsenceList.h"
#include "AbsenceDialog.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>

AbsenceList::AbsenceList(Session &session)
: session_(session)
{
    update();
}

void AbsenceList::update() {
    clear();

    auto user = session_.user();

    dbo::Transaction transaction(session_.session_);

    addWidget(std::make_unique<WText>("<h2>Abwesenheiten</h2>"));

    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Erster Tag"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Letzter Tag"));
    table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Grund"));

    auto absences = user->absences.find().orderBy("first").resultList();
    int row = 0;
    for(auto absence : absences) {
      row++;

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(absence->first.toString("yyyy-MM-dd")));
      table->elementAt(row, 2)->addWidget(std::make_unique<WText>(absence->last.toString("yyyy-MM-dd")));
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>(Absence::ReasonToString(absence->reason)));

      for(int i=0; i<4; ++i) {
        table->elementAt(row,i)->clicked().connect(this, [=] {
          absenceDialog_ = std::make_unique<AbsenceDialog>(this, session_, absence);
          absenceDialog_->show();
        });
      }
    }

    addWidget(std::move(table));

    std::string absenceButtonTitle = "Abwesenheit melden...";
    Wt::WPushButton *newAbsence = addWidget(std::make_unique<Wt::WPushButton>(absenceButtonTitle));
    newAbsence->clicked().connect(this, [=] {
       absenceDialog_ = std::make_unique<AbsenceDialog>(this, session_, nullptr);
       absenceDialog_->show();
    } );

}
