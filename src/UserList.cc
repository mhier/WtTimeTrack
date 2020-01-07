/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "UserList.h"
#include "utils.h"

#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/Auth/AbstractUserDatabase.h>

UserList::UserList(Session& session) : Updateable(nullptr), session_(session) {
  update();
}

void UserList::update() {
  clear();

  dbo::Transaction transaction(session_.session_);
  auto user = session_.user();
  if(user->role != UserRole::Admin) return;

  addWidget(std::make_unique<WText>("<h2>Benutzerverwaltung</h2>"));

  auto table = std::make_unique<WTable>();
  table->setHeaderCount(1);
  table->setWidth(WLength("100%"));
  table->addStyleClass("table form-inline table-hover");

  table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
  table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Login"));
  table->elementAt(0, 2)->addWidget(std::make_unique<WText>("E-Mail"));
  table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Rolle"));
  table->elementAt(0, 4)->addWidget(std::make_unique<WText>("Jahresbilanz"));
  table->elementAt(0, 5)->addWidget(std::make_unique<WText>("Gesamtbilanz"));

  auto users = session_.session_.find<User>().resultList();
  int row = 0;
  for(auto user : users) {
    row++;

    table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
    auto loginName = user->authInfos.front()->identity(Auth::Identity::LoginName);
    table->elementAt(row, 1)->addWidget(std::make_unique<WText>(loginName));
    table->elementAt(row, 2)->addWidget(std::make_unique<WText>(user->authInfos.front()->email()));
    if(user->role == UserRole::Admin) {
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>("Admin"));
    }
    else if(user->role == UserRole::Employee) {
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>("Mitarbeiter"));
    }
    else {
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>("???"));
    }

    auto balanceTotal = secondsToString(user->getBalanceUntil(WDate::currentDate()));
    table->elementAt(row, 4)->addWidget(std::make_unique<WText>(balanceTotal));

    WDate yearBegin(WDate::currentDate().year(), 1, 1);
    auto balanceYear = secondsToString(forUser_->getBalanceForRange(yearBegin, WDate::currentDate()));
    table->elementAt(row, 4)->addWidget(std::make_unique<WText>(balanceYear));

    for(int i = 0; i < 5; ++i) {
      table->elementAt(row, i)->clicked().connect(this, [=] {
        userDialog_ = std::make_unique<UserDialog>(this, session_, user);
        userDialog_->show();
      });
    }
  }

  addWidget(std::move(table));

  std::string buttonTitle = "Benutzer hinzufügen...";
  Wt::WPushButton* newUser = addWidget(std::make_unique<Wt::WPushButton>(buttonTitle));
  newUser->clicked().connect(this, [=] {
    userDialog_ = std::make_unique<UserDialog>(this, session_, nullptr);
    userDialog_->show();
  });
}
