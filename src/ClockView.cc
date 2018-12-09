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
    timer.setInterval(std::chrono::milliseconds(60000));
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
          {
            Wt::Dbo::Transaction transaction(session_.session_);
            session_.user().modify()->clockIn();
          }
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

    if(session_.user()->currentCreditTime().empty()) {
      todayText->setText("<p class=\"creditText paused\">"+secondsToString(credit, false)+"</p>"+
                        "<p class=\"debitText\">Soll: "+secondsToString(debit)+"</p>");
    }
    else {
      todayText->setText("<p class=\"creditText running\">"+secondsToString(credit, false)+"</p>"+
                        "<p class=\"debitText\">Soll: "+secondsToString(debit)+"</p>");
    }
    todayText->setTextFormat(Wt::TextFormat::XHTML);

    if(fullUpdate) {
      auto weekPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 2, 0);
      weekPanel->setTitle("Diese Woche");
      weekPanel->addStyleClass("panel");

      WDate first = WDate::currentDate().addDays( -(WDate::currentDate().dayOfWeek()-1) );
      WDate last = first.addDays(6);
      WDate sun = last;
      if(last > WDate::currentDate()) last = WDate::currentDate();

      auto debitW = user->getDebitForRange(first,last);
      auto debitWfull = user->getDebitForRange(first,sun);
      auto creditW = user->getCreditForRange(first,last);

      auto weekText = weekPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Ist: "+secondsToString(creditW)+"</p>"+
                                    "<p>Soll: "+secondsToString(debitW)+" (bis heute) / "+
                                    secondsToString(debitWfull)+" (ganze Woche)</p>"));
      weekText->setTextFormat(Wt::TextFormat::XHTML);

      auto monthPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 3, 0);
      monthPanel->setTitle("Dieser Monat");
      monthPanel->addStyleClass("panel");

      WDate firstDay = WDate::currentDate().addDays( -(WDate::currentDate().day()-1) );
      WDate lastDay = firstDay.addMonths(1).addDays(-1);
      WDate endOfMonth = lastDay;
      if(lastDay > WDate::currentDate()) lastDay = WDate::currentDate();

      auto debitM = user->getDebitForRange(firstDay,lastDay);
      auto debitMfull = user->getDebitForRange(firstDay,endOfMonth);
      auto creditM = user->getCreditForRange(firstDay,lastDay);

      auto monthText = monthPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Ist: "+secondsToString(creditM)+"</p>"+
                                    "<p>Soll: "+secondsToString(debitM)+" (bis heute) / "+
                                    secondsToString(debitMfull)+" (ganzer Monat)</p>"));
      monthText->setTextFormat(Wt::TextFormat::XHTML);

      auto totalPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 4, 0);
      totalPanel->setTitle("Insgesamt");
      totalPanel->addStyleClass("panel");

      auto balanceT = user->getBalanceUntil(WDate::currentDate());

      auto totalText = totalPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Bilanz: "+secondsToString(balanceT)+"</p>"));
      totalText->setTextFormat(Wt::TextFormat::XHTML);

    }

}
