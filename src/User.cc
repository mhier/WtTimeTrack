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
    if(list.size() == 0) return Wt::Dbo::ptr<Absence>(std::make_unique<Absence>());
    return list.front();
}

double User::getDebitTimeForDate(const WDate &date) const {
    auto res = debitTimes.find().where("validFrom <= ?").bind(date).orderBy("validFrom DESC").limit(1).resultList();
    if(res.size() == 0) return 0.0;
    int dayOfWeek = date.dayOfWeek()-1;     // we are counting from 0 = Monday, WDate from 1 = Monday
    return res.front()->workHoursPerWeekday[dayOfWeek];
}

int User::getCreditForRange(const WDate& from, const WDate& until) const {
    int credit;
    auto res = creditTimes.session()->query<int>("select SUM( (JULIANDAY(stop) - JULIANDAY(start))*86400. ) from creditTime")
                    .where("start >= ?").bind(from).where("stop <= ?").bind(until.addDays(1));
    return res.resultValue();
}

int User::countDebit(const Wt::Dbo::ptr<DebitTime> &debitTime, const WDate &from, const WDate &until) {
    if(debitTime.get() == nullptr) return 0;

    int nDays = from.daysTo(until)+1;       // if from and until are the same day, we count 1 day...
    int nFullWeeks = nDays / 7;             // C++ truncates towards 0
    int nDaysRemainder = nDays % 7;
    int iFirstDOW = from.dayOfWeek() - 1;   // we count from 0 = Monday

    double debitHours = 0;

    // count hours for full week
    for(int i=0; i<7; ++i) {
      debitHours += debitTime->workHoursPerWeekday[i] * nFullWeeks;
    }

    // count hours for remaining days
    for(int i=iFirstDOW; i<iFirstDOW+nDaysRemainder; ++i) {
      debitHours += debitTime->workHoursPerWeekday[i%7];
    }

    // return seconds
    return std::round(debitHours*3600.);
}

int User::getDebitForRange(const WDate& from, const WDate& until) const {
    auto list = debitTimes.find().orderBy("validFrom").resultList();
    Wt::Dbo::ptr<DebitTime> last{nullptr};
    int result = 0;
    for(auto debitTime : list) {
      if(debitTime->validFrom < from || last.get() == nullptr) {
        last = debitTime;
        continue;
      }
      WDate start = std::max( last->validFrom, from );
      WDate stop = std::min( debitTime->validFrom.addDays(-1), until );
      result += countDebit(last, start, stop);
      last = debitTime;
      if(debitTime->validFrom > until) break;
    }
    if(last.get() != nullptr && last->validFrom <= until) {
      WDate start = std::max( last->validFrom, from );
      result += countDebit(last, start, until);
    }
    return result;
}

int User::getBalanceForRange(const WDate& from, const WDate& until) const {
    auto debit = getDebitForRange(from, until);
    auto credit = getCreditForRange(from, until);
    return credit - debit;
}
