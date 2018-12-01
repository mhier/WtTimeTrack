/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "HolidayList.h"
#include "HolidayDialog.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

HolidayList::HolidayList(Session &session)
: session_(session), Updateable(nullptr)
{
    update();
}

void HolidayList::update() {
    clear();
    auto user = session_.user();
    dbo::Transaction transaction(session_.session_);
    addWidget(std::make_unique<WText>("<h2>Feiertage und Betriebsferien</h2>"));

    // Jahresauswahl
    if(year == 0) year = WDate::currentDate().year();
    addWidget(std::make_unique<Wt::WText>("Jahr wechseln: "));
    auto selectYear = addWidget(std::make_unique<Wt::WComboBox>());
    auto dt = session_.session_.find<DebitTime>().orderBy("validFrom").limit(1).resultList().front();
    int firstYear = dt->validFrom.year();
    int lastYear = WDate::currentDate().year();
    for(int i=firstYear; i<=lastYear; ++i) selectYear->addItem(std::to_string(i));
    selectYear->setCurrentIndex(year-firstYear);
    selectYear->sactivated().connect([=](WString _year){
      year = std::stoi(_year);
      update();
    });

    //Tabelle mit Feiertagen und Betriebsferien
    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Erster Tag"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Letzter Tag"));

    auto holidays = session_.session_.find<Holiday>().where("last >= ?").bind(std::to_string(year)+"-01-01").
                                                      where("first <= ?").bind(std::to_string(year)+"-12-31").
                                                      orderBy("first").resultList();
    int row = 0;
    for(auto holiday : holidays) {
      row++;

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(holiday->first.toString("yyyy-MM-dd")));
      table->elementAt(row, 2)->addWidget(std::make_unique<WText>(holiday->last.toString("yyyy-MM-dd")));

      if(user->role == UserRole::Admin) {
        for(int i=0; i<3; ++i) {
          table->elementAt(row,i)->clicked().connect(this, [=] {
            holidayDialog_ = std::make_unique<HolidayDialog>(this, session_, holiday);
            holidayDialog_->show();
          });
        }
      }
    }

    addWidget(std::move(table));

    if(user->role == UserRole::Admin) {
      std::string buttonTitle = "Feiertag/Ferien eintragen...";
      Wt::WPushButton *newHoliday = addWidget(std::make_unique<Wt::WPushButton>(buttonTitle));
      newHoliday->clicked().connect(this, [=] {
        holidayDialog_ = std::make_unique<HolidayDialog>(this, session_, nullptr);
        holidayDialog_->show();
      } );
    }

}
