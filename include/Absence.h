/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef INCLUDE_ABSENCE_H_
#define INCLUDE_ABSENCE_H_

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

class User;

class Absence {
  public:
    Absence() {}

    enum class Reason {
      NotAbsent, Holiday, SpecialLeave, Illness, OfficialTrip, Unspecified
    };

    static std::string ReasonToString(Reason reason) {
      if(reason == Reason::NotAbsent) return "anwesend";
      if(reason == Reason::Holiday) return "Urlaub";
      if(reason == Reason::SpecialLeave) return "Sonderurlaub";
      if(reason == Reason::Illness) return "Krankheit";
      if(reason == Reason::OfficialTrip) return "Dienstreise";
      return "Unbekannt";
    }

    static Reason StringToReason(const Wt::WString &string) {
      if(string == "anwesend") return Reason::NotAbsent;
      if(string == "Urlaub") return Reason::Holiday;
      if(string == "Sonderurlaub") return Reason::SpecialLeave;
      if(string == "Krankheit") return Reason::Illness;
      if(string == "Dienstreise") return Reason::OfficialTrip;
      return Reason::Unspecified;
    }

    Dbo::ptr<User> user;

    WDate first;
    WDate last;
    Reason reason{Reason::NotAbsent};

    template<class Action>
    void persist ( Action& a ) {
        Dbo::belongsTo ( a, user, "absences" );

        Dbo::field ( a, first, "first" );
        Dbo::field ( a, last, "last" );
        Dbo::field ( a, reason, "reason" );
    }

};

#endif /* INCLUDE_ABSENCE_H_ */
