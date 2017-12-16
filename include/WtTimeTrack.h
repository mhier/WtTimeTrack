/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANGAME_H_
#define HANGMANGAME_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WMenu.h>

#include "Session.h"

using namespace Wt;

namespace Wt {
  class WStackedWidget;
  class WAnchor;
}

class Session;

class WtTimeTrack : public WContainerWidget {
  public:
    WtTimeTrack();

    void handleInternalPath(const std::string &internalPath);

  private:
    WStackedWidget    *mainStack_{nullptr};
    WStackedWidget    *contentStack_{nullptr};

    Session session_;

    void onAuthEvent();
    void createMenu();
    void monthView();
    void absencesView();
    void clockView();
    void debitTimeView();

    void clockIn();
    void clockOut();
};

#endif //HANGMANGAME_H_
