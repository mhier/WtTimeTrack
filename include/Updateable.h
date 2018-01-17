/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef UPDATEABLE_H_
#define UPDATEABLE_H_

#include "User.h"

class Updateable {

  public:

    Updateable(Wt::Dbo::ptr<User> forUser)
    : forUser_(forUser) {}

    virtual ~Updateable() {}

    virtual void update() = 0;

    Wt::Dbo::ptr<User> forUser_;

};

#endif // UPDATEABLE_H_
