/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef ANNUALSTATEMENT_H
#define ANNUALSTATEMENT_H

#include <Wt/WDate.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class User;

class AnnualStatement {
  public:
    AnnualStatement() {}

    Dbo::ptr<User> user;
    Wt::WDate referenceDate;
    int balance;        // as returned by getBalanceUntil() for the referenceDate

    template<class Action>
    void persist ( Action& a ) {
        Dbo::belongsTo ( a, user, "annualStatements" );
        Wt::Dbo::field ( a, referenceDate, "referenceDate" );
        Wt::Dbo::field ( a, balance, "balance" );
    }

};

#endif // ANNUALSTATEMENT_H
