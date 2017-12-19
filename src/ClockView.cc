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
          transaction.commit();
          update();
        });
      }
      else {
        auto button = clockInOutPanel->setCentralWidget(std::make_unique<Wt::WPushButton>("Ausstempeln"));
        button->clicked().connect(this, [=] {
          Wt::Dbo::Transaction transaction(session_.session_);
          session_.user().modify()->clockOut();
          transaction.commit();
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
      auto weekPanel = layout->addWidget(std::make_unique<Wt::WPanel>(), 2, 0);
      weekPanel->setTitle("Diese Woche");
      weekPanel->addStyleClass("panel");

      WDate mon = WDate::currentDate();
      mon.addDays( -(mon.dayOfWeek()-1) );
      WDate sun = mon.addDays(7);
      auto debitW = user->getDebitForRange(mon,sun);
      auto creditW = user->getCreditForRange(mon,sun);

      auto weekText = weekPanel->setCentralWidget(
        std::make_unique<Wt::WText>("<p>Ist: "+secondsToString(creditW)+"</p><p>Soll: "+secondsToString(debitW)+"</p>"));
      weekText->setTextFormat(Wt::TextFormat::XHTML);
    }

}
