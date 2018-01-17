/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "MonthView.h"
#include "utils.h"
#include "PlannerCalendar.h"

#include <Wt/WText.h>
#include <Wt/WTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/WPanel.h>
#include <Wt/WComboBox.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>

MonthView::MonthView(Session &session, Wt::Dbo::ptr<User> forUser)
: session_(session), Updateable(forUser)
{
    update();
}

void MonthView::update() {
    auto user = session_.user();
    Wt::Dbo::Transaction transaction(session_.session_);

    auto layout = setLayout(std::make_unique<Wt::WGridLayout>());

    if(user->role == UserRole::Admin) {
      auto changeUser = new Wt::WGridLayout();
      changeUser->addWidget(std::make_unique<Wt::WText>("Mitarbeiter wechseln: "), 0, 0);
      auto cb = changeUser->addWidget(std::make_unique<Wt::WComboBox>(), 0, 1 );
      cb->setWidth(200);
      layout->addLayout( std::unique_ptr<Wt::WLayout>(changeUser), 0, 0 );

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

    layout->addWidget( std::make_unique<PlannerCalendar>(session_, *this), 1, 0 );

    auto weekSummary = new Wt::WGridLayout();
    weekSummary->setVerticalSpacing(0);
    weekSummary->addWidget( std::make_unique<Wt::WText>(""), 0, 0 );
    for(int i=0; i<6; ++i) {
      weekSummaryTexts[i] = weekSummary->addWidget( std::make_unique<Wt::WText>(""), 1+i, 0 );
      weekSummaryTexts[i]->setHeight(80);
      weekSummaryTexts[i]->setStyleClass("week-summary");
      weekSummary->setRowStretch(1+i, 0);
    }
    weekSummary->setRowStretch(0, 1);
    layout->addLayout( std::unique_ptr<Wt::WLayout>(weekSummary), 1, 1 );

    monthSummaryText = layout->addWidget( std::make_unique<Wt::WText>(""), 2, 0 );
    monthSummaryText->setStyleClass("month-summary");

}

