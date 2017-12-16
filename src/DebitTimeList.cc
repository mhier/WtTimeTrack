/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <iomanip>

#include "DebitTimeList.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>

std::array<std::string, 7> const DebitTimeList::dayOfWeekNames{{"Mo", "Di", "Mi", "Do", "Fr",
                                                                "Sa", "So"}};

DebitTimeList::DebitTimeList(Session &session)
: session_(session)
{
    update();
}

void DebitTimeList::update() {
    clear();

    auto user = session_.user();

    dbo::Transaction transaction(session_.session_);

    addWidget(std::make_unique<WText>("<h2>Arbeitsstunden</h2>"));

    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Gültig ab"));
    for(size_t i=0; i<7; ++i) table->elementAt(0, 2+i)->addWidget(std::make_unique<WText>(dayOfWeekNames[i]));
    table->elementAt(0, 9)->addWidget(std::make_unique<WText>("Summe"));

    auto debitTimes = user->debitTimes.find().orderBy("validFrom").resultList();
    int row = 0;
    for(auto debitTime : debitTimes) {
      row++;

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(debitTime->validFrom.toString("yyyy-MM-dd")));
      double sum = 0;
      for(size_t i=0; i<7; ++i) {
        sum += debitTime->workHoursPerWeekday[i];
        table->elementAt(row, 2+i)->addWidget(std::make_unique<WText>( formatNumber(debitTime->workHoursPerWeekday[i]) ));
      }
      table->elementAt(row, 9)->addWidget(std::make_unique<WText>(formatNumber(sum)));

      for(int i=0; i<4; ++i) {
        table->elementAt(row,i)->clicked().connect(this, [=] {
          debitTimeDialog_ = std::make_unique<DebitTimeDialog>(this, session_, debitTime);
          debitTimeDialog_->show();
        });
      }
    }

    addWidget(std::move(table));

    std::string buttonTitle = "Arbeitszeitänderung eintragen...";
    Wt::WPushButton *newDebitTime = addWidget(std::make_unique<Wt::WPushButton>(buttonTitle));
    newDebitTime->clicked().connect(this, [=] {
       debitTimeDialog_ = std::make_unique<DebitTimeDialog>(this, session_, nullptr);
       debitTimeDialog_->show();
    } );


    transaction.commit();

}

std::string DebitTimeList::formatNumber(double hours) const {
    std::ostringstream oss;
    oss << std::setprecision(2) << std::noshowpoint << hours;
    return oss.str();
}
