/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "User.h"

#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(User);

using namespace Wt::Dbo;


collection< ptr<CreditTime> > User::creditTimesInRange(const WDate& from, const WDate& until) const {
    return creditTimes.find()
      .where("start >= ?").bind(WDateTime(from))
      .where("start < ?").bind(WDateTime(until));
}

collection< ptr<CreditTime> > User::currentCreditTime() const {
    return creditTimes.find().where("hasClockedOut == 0");
}

void User::clockIn() {
    if(!currentCreditTime().empty()) return;    // already clocked in: ignore

    Wt::Dbo::ptr<CreditTime> time(std::make_unique<CreditTime>());
    time.modify()->start = WDateTime::currentDateTime();
    time.modify()->stop = WDateTime();

    creditTimes.insert(time);

}

void User::clockOut() {
    auto current = currentCreditTime();
    if(current.empty()) return;     // not clocked in: ignore
    auto mCurrent = current.front().modify();
    mCurrent->stop = WDateTime::currentDateTime();
    mCurrent->hasClockedOut = true;

}

Wt::Dbo::ptr<Absence> User::checkAbsence(const WDate& date) const {
    auto res = absences.find().where("first <= ?").bind(date)
                              .where("last >= ?").bind(date);

    auto list = res.resultList();
    int sss = list.size();
    if(sss == 0) return Wt::Dbo::ptr<Absence>(std::make_unique<Absence>());
    return list.front();
}
