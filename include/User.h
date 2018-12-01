/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef USER_H_
#define USER_H_

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <string>
#include <list>

#include "CreditTime.h"
#include "DebitTime.h"
#include "Absence.h"
#include "AnnualStatement.h"

using namespace Wt;

namespace dbo = Wt::Dbo;

class User;
class Session;
typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef dbo::collection< dbo::ptr<User> > Users;

enum class UserRole {
    Employee=0, Admin=1
};

class User {
  public:
    User() { invalidateCaches(); }
    virtual ~User() {}

    std::string name; /* a copy of auth info's user name */

    UserRole role {UserRole::Employee};

    dbo::collection<dbo::ptr<AuthInfo>> authInfos;

    dbo::collection< dbo::ptr<CreditTime> > creditTimes;
    dbo::collection< dbo::ptr<CreditTime> > creditTimesInRange(const WDate& from, const WDate& until) const;
    dbo::collection< dbo::ptr<CreditTime> > currentCreditTime() const;

    dbo::collection< dbo::ptr<DebitTime> > debitTimes;

    // obtain a list of DebitTimes taking into account the Absences. Result is ordered by "validFrom"
    std::list<DebitTime> getDebitTimesWithAbsences(bool includeVacation = true) const;

    // credit time in seconds for a given range (including both dates)
    int getCreditForRange(const WDate& from, const WDate& until) const;

    // debit time in seconds for a given range (including both dates)
    int getDebitForRange(const WDate& from, const WDate& until, bool includeVacation = true) const;

    // debit time for given date in seconds (more efficient than getDebitTimeForRange() in case of a single day)
    int getDebitTimeForDate(const WDate &date) const;

    // in seconds. positive: extra hours
    int getBalanceForRange(const WDate& from, const WDate& until) const;
    int getBalanceUntil(const WDate& date, bool useStatementForExactDate=true) const;

    dbo::collection< dbo::ptr<Absence> > absences;
    Wt::Dbo::ptr<Absence> checkAbsence(const WDate& date) const;
    int countHolidays(const WDate& from, const WDate& until) const;

    void clockIn();
    void clockOut();

    dbo::collection< dbo::ptr<AnnualStatement> > annualStatements;

    template<class Action>
    void persist ( Action& a ) {
      dbo::field ( a, role, "role" );
      dbo::field ( a, name, "name" );

      dbo::hasMany ( a, authInfos, dbo::ManyToOne, "user" );
      dbo::hasMany ( a, creditTimes, dbo::ManyToOne, "creditTimes" );
      dbo::hasMany ( a, debitTimes, dbo::ManyToOne, "debitTimes" );
      dbo::hasMany ( a, absences, dbo::ManyToOne, "absences" );
      dbo::hasMany ( a, annualStatements, dbo::ManyToOne, "annualStatements" );
    }

    void invalidateCaches() const {
      static const WDateTime invalidTime(WDateTime::currentDateTime().addDays(-1));
      age_debitTimesWithAbsencesInclAbsences = invalidTime;
      age_debitTimesWithAbsencesExclAbsences = invalidTime;
    }

  private:

    // count debit time in seconds in the given range assuming the given debitTime for each week day.
    // DebitTime::validFrom is ignored in this function as well as absences!
    static int countDebit(const DebitTime &debitTime, const WDate &from, const WDate &until);

    // count days with non-zero debit time in the given range assuming the given debitTime for each week day.
    // DebitTime::validFrom is ignored in this function as well as absences!
    static int countDebitDays(const DebitTime &debitTime, const WDate &from, const WDate &until);

    // caches for getDebitTimesWithAbsences()
    mutable WDateTime age_debitTimesWithAbsencesInclAbsences;
    mutable std::list<DebitTime> cache_debitTimesWithAbsencesInclAbsences;
    mutable WDateTime age_debitTimesWithAbsencesExclAbsences;
    mutable std::list<DebitTime> cache_debitTimesWithAbsencesExclAbsences;
};

DBO_EXTERN_TEMPLATES ( User );

#endif // USER_H_
