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

PlannerCalendar::PlannerCalendar(Session &session)
  : WCalendar(),
    session_(session)
{
  setStyleClass(styleClass() + " calendar");

  setSelectionMode(SelectionMode::None);
}

WWidget* PlannerCalendar::renderCell(WWidget* widget, const WDate& date) {
  if(!widget) widget = new CalendarCell(session_);

  CalendarCell* cc = (CalendarCell*)widget;
  cc->update(date);
  cc->owner_ = this;

  return cc;
}
