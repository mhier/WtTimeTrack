/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/RegistrationModel.h>
#include <Wt/WText.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WCalendar.h>
#include <Wt/WPushButton.h>

#include "WtTimeTrack.h"
#include "MonthView.h"
#include "AbsenceList.h"
#include "DebitTimeList.h"
#include "ClockView.h"
#include "HolidayList.h"
#include "UserList.h"

WtTimeTrack::WtTimeTrack() {
    session_.login().changed().connect(this, &WtTimeTrack::onAuthEvent);

    auto authModel = std::make_unique<Auth::AuthModel>(Session::auth(), session_.users());
    authModel->addPasswordAuth(&Session::passwordAuth());

    auto authWidget = std::make_unique<Auth::AuthWidget>(session_.login());
    auto authWidgetPtr = authWidget.get();
    authWidget->setModel(std::move(authModel));
    authWidget->setRegistrationEnabled(false);

    std::unique_ptr<WText> title(std::make_unique<WText>("<h1>Zeiterfassung</h1>"));
    addWidget(std::move(title));

    addWidget(std::move(authWidget));

    mainStack_ = new WStackedWidget();
    mainStack_->setHeight("100vH");
    addWidget(std::unique_ptr<WStackedWidget>(mainStack_));

    WApplication::instance()->internalPathChanged().connect(this, &WtTimeTrack::handleInternalPath);

    authWidgetPtr->processEnvironment();
}

void WtTimeTrack::onAuthEvent() {
    if(session_.login().loggedIn()) {
      createMenu();
      handleInternalPath(WApplication::instance()->internalPath());
    }
    else {
      mainStack_->clear();
      WApplication::instance()->setInternalPath("/",  true);
    }
}

void WtTimeTrack::handleInternalPath(const std::string &internalPath) {
    if(session_.login().loggedIn()) {
      if (internalPath == "/month") {
        monthView();
      }
      else if (internalPath == "/absences") {
        absencesView();
      }
      else if (internalPath == "/clock") {
        clockView();
      }
      else if (internalPath == "/debitTimes") {
        debitTimeView();
      }
      else if (internalPath == "/holidays") {
        holidayView();
      }
      else if (internalPath == "/users" && session_.user()->role == UserRole::Admin) {
        userView();
      }
      else {
        WApplication::instance()->setInternalPath("/clock",  true);
      }
    }
    else {
      WApplication::instance()->setInternalPath("/",  true);
    }
}

void WtTimeTrack::createMenu() {
    Wt::Dbo::Transaction transaction(session_.session_);

    auto pane = std::make_unique<Wt::WContainerWidget>();
    auto pane_ = pane.get();
    mainStack_->addWidget(std::move(pane));
    auto hLayout = pane_->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hLayout->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);

    auto contentStack = std::make_unique<Wt::WStackedWidget>();
    contentStack_ = contentStack.get();
    contentStack_->addStyleClass("contents");
    contentStack_->setOverflow(Wt::Overflow::Auto);

    auto menu = std::make_unique<Wt::WMenu>(contentStack_);
    auto menu_ = menu.get();
    menu_->addStyleClass("nav nav-pills nav-stacked");
    menu_->setWidth(200);

    hLayout->addWidget(std::move(menu));
    hLayout->addWidget(std::move(contentStack),1);

    menu_->setInternalPathEnabled();
    menu_->setInternalBasePath("/");

    menu_->addItem("Ein-/Ausstempeln")->setPathComponent("clock");
    menu_->addItem("Monatsansicht")->setPathComponent("month");
    menu_->addItem("Abwesenheit")->setPathComponent("absences");
    menu_->addItem("Arbeitszeiten")->setPathComponent("debitTimes");
    menu_->addItem("Feiertage/Ferien")->setPathComponent("holidays");
    if(session_.user()->role == UserRole::Admin) {
      menu_->addItem("Benutzerverwaltung")->setPathComponent("users");
    }

}

void WtTimeTrack::monthView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<MonthView>( session_ ));

}

void WtTimeTrack::absencesView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<AbsenceList>(session_));

}

void WtTimeTrack::debitTimeView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<DebitTimeList>(session_, session_.user()));

}

void WtTimeTrack::clockView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<ClockView>(session_));

}

void WtTimeTrack::holidayView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<HolidayList>(session_));

}

void WtTimeTrack::userView() {

    Wt::Dbo::Transaction transaction(session_.session_);
    contentStack_->clear();
    contentStack_->addWidget(std::make_unique<UserList>(session_));

}
