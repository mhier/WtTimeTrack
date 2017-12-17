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

#include "CreditTime.h"
#include "DebitTime.h"
#include "Absence.h"

using namespace Wt;

namespace dbo = Wt::Dbo;

class User;
class Session;
typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef dbo::collection< dbo::ptr<User> > Users;

enum class UserRole {
    Employee, Admin
};

class User {
  public:
    User() {}
    virtual ~User() {}

    std::string name; /* a copy of auth info's user name */

    UserRole role {UserRole::Employee};

    dbo::collection<dbo::ptr<AuthInfo>> authInfos;

    dbo::collection< dbo::ptr<CreditTime> > creditTimes;
    dbo::collection< dbo::ptr<CreditTime> > creditTimesInRange(const WDate& from, const WDate& until) const;
    dbo::collection< dbo::ptr<CreditTime> > currentCreditTime() const;

    dbo::collection< dbo::ptr<DebitTime> > debitTimes;
    double getDebitTimeForDate(const WDate &date) const;

    // credit time in seconds for a given range
    int getCreditForRange(const WDate& from, const WDate& until) const;
    // debit time in seconds for a given range
    int getDebitForRange(const WDate& from, const WDate& until) const;

    // in seconds. positive: extra hours
    int getBalanceForRange(const WDate& from, const WDate& until) const;
    int getBalanceUntil(const WDate& date) const {
      return getBalanceForRange(WDate(1970,1,1), date);
    }

    dbo::collection< dbo::ptr<Absence> > absences;
    Wt::Dbo::ptr<Absence> checkAbsence(const WDate& date) const;

    void clockIn();
    void clockOut();

    template<class Action>
    void persist ( Action& a ) {
      dbo::field ( a, role, "role" );

      dbo::hasMany ( a, authInfos, dbo::ManyToOne, "user" );
      dbo::hasMany ( a, creditTimes, dbo::ManyToOne, "creditTimes" );
      dbo::hasMany ( a, debitTimes, dbo::ManyToOne, "debitTimes" );
      dbo::hasMany ( a, absences, dbo::ManyToOne, "absences" );
    }

  private:

    // count debit time in seconds in the given range assuming the given debitTime for each week day.
    // DebitTime::validFrom is ignored in this function!
    static int countDebit(const Wt::Dbo::ptr<DebitTime> &debitTime, const WDate &from, const WDate &until);
};

DBO_EXTERN_TEMPLATES ( User );

#endif // USER_H_
