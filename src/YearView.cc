/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "YearView.h"
#include "utils.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>

std::array<std::string, 12> const YearView::monthNames{{"Januar", "Februar", "März", "April", "Mai", "Juni", "July",
                                                        "August", "September", "Oktober", "November", "Dezember"}};

YearView::YearView(Session &session, Wt::Dbo::ptr<User> forUser)
: session_(session), forUser_(forUser)
{
    update();
}

void YearView::update() {
    clear();

    auto user = session_.user();

    dbo::Transaction transaction(session_.session_);

    addWidget(std::make_unique<WText>("<h2>Jahresübersicht für '"+forUser_->name+"'</h2>"));

    int year = WDate::currentDate().year();   /// @todo make selectable

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

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("Monat"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Urlaubstage"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Soll-Stunden"));
    table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Ist-Stunden"));
    table->elementAt(0, 4)->addWidget(std::make_unique<WText>("Bilanz"));
    table->elementAt(0, 5)->addWidget(std::make_unique<WText>("Bilanz seit Jahresanfang"));

    WDate yearBegin(year, 1, 1);
    for(int row=1; row<=12; ++row) {
      WDate first(year, row, 1);
      WDate last = first.addMonths(1).addDays(-1);
      std::string holidays = std::to_string(forUser_->countHolidays(first, last));
      std::string debit = secondsToString(forUser_->getDebitForRange(first, last));
      std::string credit = secondsToString(forUser_->getCreditForRange(first, last));
      std::string balance = secondsToString(forUser_->getBalanceForRange(first, last));
      std::string balanceYear = secondsToString(forUser_->getBalanceForRange(yearBegin, last));

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(monthNames[row-1]));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(holidays));
      table->elementAt(row, 2)->addWidget(std::make_unique<WText>(debit));
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>(credit));
      table->elementAt(row, 4)->addWidget(std::make_unique<WText>(balance));
      table->elementAt(row, 5)->addWidget(std::make_unique<WText>(balanceYear));
    }

    WDate yearEnd = yearBegin.addYears(1).addDays(-1);

    std::string holidays = std::to_string(forUser_->countHolidays(yearBegin, yearEnd));
    std::string debit = secondsToString(forUser_->getDebitForRange(yearBegin, yearEnd));
    std::string credit = secondsToString(forUser_->getCreditForRange(yearBegin, yearEnd));
    std::string balance = secondsToString(forUser_->getBalanceForRange(yearBegin, yearEnd));
    std::string balanceYear = secondsToString(forUser_->getBalanceForRange(yearBegin, yearEnd));

    table->elementAt(13, 0)->addWidget(std::make_unique<WText>("Summe"));
    table->elementAt(13, 1)->addWidget(std::make_unique<WText>(holidays));
    table->elementAt(13, 2)->addWidget(std::make_unique<WText>(debit));
    table->elementAt(13, 3)->addWidget(std::make_unique<WText>(credit));
    table->elementAt(13, 4)->addWidget(std::make_unique<WText>(balance));
    table->elementAt(13, 5)->addWidget(std::make_unique<WText>(balanceYear));

    addWidget(std::move(table));

}

