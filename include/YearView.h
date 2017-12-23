/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef YEAR_VIEW_H_
#define YEAR_VIEW_H_

#include "User.h"
#include "Session.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class YearView : public WContainerWidget {
  public:
    YearView(Session &session, Wt::Dbo::ptr<User> forUser);

    Session &session_;

    void update();

    static const std::array<std::string, 12> monthNames;

    Wt::Dbo::ptr<User> forUser_;
};

#endif //YEAR_VIEW_H_
