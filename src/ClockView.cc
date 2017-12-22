/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "ClockView.h"
#include "utils.h"

#include <Wt/WText.h>
#include <Wt/WTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/WPanel.h>

ClockView::ClockView(Session &session)
: session_(session)
{
    update();
    timer.setInterval(std::chrono::milliseconds(1000));
    timer.timeout().connect(this, [=] { update(); });
    timer.start();
}

void ClockView::update(bool fullUpdate) {

    auto user = session_.user();
    Wt::Dbo::Transaction transaction(session_.session_);

    Wt::WGridLayout *layout;
    if(fullUpdate) {
      clear();

      layout = setLayout(std::make_unique<Wt::WGridLayout>());


      auto clockInOutPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 0, 0);
      clockInOutPanel->setTitle("Ein-/Ausstempeln");
      clockInOutPanel->addStyleClass("panel");

      if(session_.user()->currentCreditTime().empty()) {
        auto button = clockInOutPanel->setCentralWidget(std::make_unique<Wt::WPushButton>("Einstempeln"));
        button->clicked().connect(this, [=] {
          Wt::Dbo::Transaction transaction(session_.session_);
          session_.user().modify()->clockIn();
          update();
        });
      }
      else {
        auto button = clockInOutPanel->setCentralWidget(std::make_unique<Wt::WPushButton>("Ausstempeln"));
        button->clicked().connect(this, [=] {
          Wt::Dbo::Transaction transaction(session_.session_);
          session_.user().modify()->clockOut();
          update();
        });
      }

      auto todayPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 1, 0);
      todayPanel->setTitle("Heute: "+WDate::currentDate().toString("yyyy-MM-dd"));
      todayPanel->addStyleClass("panel");

      todayText = todayPanel->setCentralWidget(std::make_unique<Wt::WText>());
    }

    auto debit = user->getDebitTimeForDate(WDate::currentDate());
    auto credit = user->getCreditForRange(WDate::currentDate(), WDate::currentDate());

    todayText->setText("<p class=\"creditText\">"+secondsToString(credit, true)+"</p>"+
                       "<p class=\"debitText\">Soll: "+secondsToString(debit)+"</p>");
    todayText->setTextFormat(Wt::TextFormat::XHTML);

    if(fullUpdate) {
      std::cout << "HIER 1" << std::endl;
      auto weekPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 2, 0);
      weekPanel->setTitle("Diese Woche");
      weekPanel->addStyleClass("panel");

      std::cout << "HIER 2" << std::endl;
      WDate mon = WDate::currentDate().addDays( -(WDate::currentDate().dayOfWeek()-1) );
      std::cout << "HIER 3" << std::endl;
      WDate sun = mon.addDays(7);
      std::cout << "HIER 4" << std::endl;
      auto debitW = user->getDebitForRange(mon,sun);
      auto creditW = user->getCreditForRange(mon,sun);

      auto weekText = weekPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Ist: "+secondsToString(creditW)+"</p><p>Soll: "+secondsToString(debitW)+"</p>"));
      weekText->setTextFormat(Wt::TextFormat::XHTML);

      std::cout << "HIER 5" << std::endl;
      auto monthPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 3, 0);
      monthPanel->setTitle("Dieser Monat");
      monthPanel->addStyleClass("panel");

      std::cout << "HIER 6" << std::endl;
      WDate firstDay = WDate::currentDate().addDays( -(WDate::currentDate().day()-1) );
      std::cout << "HIER 7" << std::endl;
      WDate lastDay = firstDay.addMonths(1).addDays(-1);
      std::cout << "HIER 8" << std::endl;
      auto debitM = user->getDebitForRange(firstDay,lastDay);
      auto creditM = user->getCreditForRange(firstDay,lastDay);

      std::cout << "HIER 9" << std::endl;
      auto monthText = monthPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Ist: "+secondsToString(creditM)+"</p><p>Soll: "+secondsToString(debitM)+"</p>"));
      std::cout << "HIER 10" << std::endl;
      monthText->setTextFormat(Wt::TextFormat::XHTML);
      std::cout << "HIER 11" << std::endl;

    }

}
