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

int User::getDebitTimeForDate(const WDate &date) const {
    auto list = getDebitTimesWithAbsences();
    for(auto it = list.rbegin(); it != list.rend(); ++it) {
      if(it->validFrom <= date) {
        return it->workHoursPerWeekday[ date.dayOfWeek()-1 ] * 3600;
      }
    }
}

int User::getCreditForRange(const WDate& from, const WDate& until) const {
    int credit = 0;

    auto res = creditTimes.find().where("start >= ?").bind(from)
                                 .where("start <= ?").bind(until.addDays(1));
    for(auto time : res.resultList()) {
      credit += time->getSecs();
    }

    return credit;
}

int User::countDebit(const DebitTime &debitTime, const WDate &from, const WDate &until) {

    int nDays = from.daysTo(until)+1;       // if from and until are the same day, we count 1 day...
    int nFullWeeks = nDays / 7;             // C++ truncates towards 0
    int nDaysRemainder = nDays % 7;
    int iFirstDOW = from.dayOfWeek() - 1;   // we count from 0 = Monday

    double debitHours = 0;

    // count hours for full week
    for(int i=0; i<7; ++i) {
      debitHours += debitTime.workHoursPerWeekday[i] * nFullWeeks;
    }

    // count hours for remaining days
    for(int i=iFirstDOW; i<iFirstDOW+nDaysRemainder; ++i) {
      debitHours += debitTime.workHoursPerWeekday[i%7];
    }

    // return seconds
    return std::round(debitHours*3600.);
}

std::list<DebitTime> User::getDebitTimesWithAbsences() const {

    // put the debit times into a std::list
    auto debitCollection = debitTimes.find().orderBy("validFrom").resultList();
    std::list<DebitTime> debitList;
    for(auto &debit : debitCollection) debitList.push_back(*debit);

    // iterate through absences in reverse order and insert into debitList
    auto absenceCollection = absences.find().orderBy("first DESC").resultList();
    for(auto &absence : absenceCollection) {
      WDate validUntil = WDate::currentDate().addYears(100);    // 100 years in future: latest debitTime expires
      for(auto it = debitList.rbegin(); it != debitList.rend(); ++it) {
        if(it->validFrom > absence->last || validUntil < absence->first) {
          validUntil = it->validFrom.addDays(-1);
          continue;
        }
        // in general we will effectivly replace the debitTime in the list with 3 consecutive debitTimes:
        // the first will be identical to the original one, the second will reflect the absence (debit hours
        // are all 0) and the third will restore the original work hours.
        DebitTime debit1 = *it;
        DebitTime debit2;
        debit2.validFrom = absence->first;
        it->validFrom = absence->last.addDays(1);
        auto insertBefore = it.base();
        insertBefore--;
        // the first debit time only gets inserted if positive length
        if(debit1.validFrom < debit2.validFrom) debitList.insert(insertBefore, debit1);
        // the second one gets always inserted as it represents the absence itself
        debitList.insert(insertBefore, debit2);
        // the third gets removed if it no longer has a positive length
        if(it->validFrom > validUntil) debitList.erase(insertBefore);
        break;
      }
    }

    return debitList;
}

int User::getDebitForRange(const WDate& from, const WDate& until) const {
    auto list = getDebitTimesWithAbsences();
    int result = 0;
    WDate validUntil = WDate::currentDate().addYears(100);    // 100 years in future: latest debitTime expires
    for(auto it = list.rbegin(); it != list.rend(); ++it) {
      if(it->validFrom > until) {
        validUntil = it->validFrom.addDays(-1);
        continue;
      }
      WDate start = std::max( it->validFrom, from );
      WDate stop = std::min( validUntil, until );
      result += countDebit(*it, start, stop);
      if(it->validFrom < from) break;
      validUntil = it->validFrom.addDays(-1);
    }
    return result;
}

int User::getBalanceForRange(const WDate& from, const WDate& until) const {
    auto debit = getDebitForRange(from, until);
    auto credit = getCreditForRange(from, until);
    return credit - debit;
}
