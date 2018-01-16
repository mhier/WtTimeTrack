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
#include <Wt/WComboBox.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>

std::array<std::string, 7> const DebitTimeList::dayOfWeekNames{{"Mo", "Di", "Mi", "Do", "Fr",
                                                                "Sa", "So"}};

DebitTimeList::DebitTimeList(Session &session, Wt::Dbo::ptr<User> forUser)
: session_(session), forUser_(forUser)
{
    update();
}

void DebitTimeList::update() {
    clear();

    auto user = session_.user();

    dbo::Transaction transaction(session_.session_);

    addWidget(std::make_unique<WText>("<h2>Arbeitsstunden für '"+forUser_->name+"'</h2>"));

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

    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Gültig ab"));
    for(size_t i=0; i<7; ++i) table->elementAt(0, 2+i)->addWidget(std::make_unique<WText>(dayOfWeekNames[i]));
    table->elementAt(0, 9)->addWidget(std::make_unique<WText>("Summe"));

    auto debitTimes = forUser_->debitTimes.find().orderBy("validFrom").resultList();
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

      if(user->role == UserRole::Admin) {
        for(int i=0; i<10; ++i) {
          table->elementAt(row,i)->clicked().connect(this, [=] {
            debitTimeDialog_ = std::make_unique<DebitTimeDialog>(this, session_, debitTime, forUser_);
            debitTimeDialog_->show();
          });
        }
      }
    }

    addWidget(std::move(table));

    if(user->role == UserRole::Admin) {
      std::string buttonTitle = "Arbeitszeitänderung eintragen...";
      Wt::WPushButton *newDebitTime = addWidget(std::make_unique<Wt::WPushButton>(buttonTitle));
      newDebitTime->clicked().connect(this, [=] {
        debitTimeDialog_ = std::make_unique<DebitTimeDialog>(this, session_, nullptr, forUser_);
        debitTimeDialog_->show();
      } );
    }

}

std::string DebitTimeList::formatNumber(double hours) {
    char buffer[6];
    sprintf(buffer, "%5.2f", hours);
    return buffer;
}
