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
#include <Wt/WComboBox.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>

AbsenceList::AbsenceList(Session &session,  Wt::Dbo::ptr<User> forUser)
: session_(session), Updateable(forUser)
{
    update();
}

void AbsenceList::update() {
    clear();
    addWidget(std::make_unique<WText>("<h2>Abwesenheiten</h2>"));
    dbo::Transaction transaction(session_.session_);

    // Jahresauswahl
    if(year == 0) year = WDate::currentDate().year();
    addWidget(std::make_unique<Wt::WText>("Jahr wechseln: "));
    auto selectYear = addWidget(std::make_unique<Wt::WComboBox>());
    auto dt = forUser_.get()->debitTimes.find().orderBy("validFrom").limit(1).resultList().front();
    int firstYear = dt->validFrom.year();
    int lastYear = WDate::currentDate().year();
    for(int i=firstYear; i<=lastYear; ++i) selectYear->addItem(std::to_string(i));
    selectYear->setCurrentIndex(year-firstYear);
    selectYear->sactivated().connect([=](WString _year){
      year = std::stoi(_year);
      update();
    });

    // Mitarbeiterauswahl (falls Admin)
    auto user = session_.user();
    if(user->role == UserRole::Admin) {
      addWidget(std::make_unique<Wt::WText>("Mitarbeiter wechseln: "));
      auto cb = addWidget(std::make_unique<Wt::WComboBox>());

      auto users = session_.session_.find<User>().resultList();
      int idx = 0;
      for(auto u : users) {
        auto loginName = u->authInfos.front()->identity(Auth::Identity::LoginName);
        cb->addItem(loginName);
        if(u == forUser_) cb->setCurrentIndex(idx);
        ++idx;
      }
      cb->sactivated().connect([=](WString name) {
        dbo::Transaction transaction(session_.session_);
        auto list = session_.session_.find<User>().where("name = ?").bind(name).resultList();
        if(list.empty()) return;    /// @todo make error message
        forUser_ = list.front();
        update();
      });
    }

    // Tabelle mit Abwesenheiten
    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Erster Tag"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Letzter Tag"));
    table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Grund"));

    auto absences = forUser_->absences.find().where("last >= ?").bind(std::to_string(year)+"-01-01").
                                              where("first <= ?").bind(std::to_string(year)+"-12-31").
                                              orderBy("first").resultList();
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
