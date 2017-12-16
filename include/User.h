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
};

DBO_EXTERN_TEMPLATES ( User );

#endif // USER_H_
