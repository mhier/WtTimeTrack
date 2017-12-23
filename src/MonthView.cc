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

MonthView::MonthView(Session &session)
: session_(session)
{
    auto user = session_.user();
    Wt::Dbo::Transaction transaction(session_.session_);

    auto layout = setLayout(std::make_unique<Wt::WGridLayout>());
    layout->addWidget( std::make_unique<PlannerCalendar>(session_, *this), 0, 0 );

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
    layout->addLayout( std::unique_ptr<Wt::WLayout>(weekSummary), 0, 1 );

    monthSummaryText = layout->addWidget( std::make_unique<Wt::WText>(""), 1, 0 );
    monthSummaryText->setStyleClass("month-summary");

}

