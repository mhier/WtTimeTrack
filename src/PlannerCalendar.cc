/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "PlannerCalendar.h"
#include "CalendarCell.h"

PlannerCalendar::PlannerCalendar(Session &session, MonthView &view)
  : WCalendar(),
    session_(session),
    view_(view)
{
  setStyleClass(styleClass() + " calendar");
  setSelectionMode(SelectionMode::None);
}

WWidget* PlannerCalendar::renderCell(WWidget* widget, const WDate& date) {
  if(!widget) widget = new CalendarCell(session_);

  CalendarCell* cc = (CalendarCell*)widget;
  cc->owner_ = this;
  cc->update(date);

  return cc;
}
