/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "YearView.h"
#include "utils.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WMessageBox.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>

std::array<std::string, 12> const YearView::monthNames{{"Januar", "Februar", "März", "April", "Mai", "Juni", "July",
                                                        "August", "September", "Oktober", "November", "Dezember"}};

YearView::YearView(Session &session, Wt::Dbo::ptr<User> forUser)
: session_(session), forUser_(forUser)
{
    update();
}

void YearView::update() {
    clear();

    auto user = session_.user();
    dbo::Transaction transaction(session_.session_);
    addWidget(std::make_unique<WText>("<h2>Jahresübersicht für '"+forUser_->name+"'</h2>"));

    // Jahresauswahl
    if(year == 0) year = WDate::currentDate().year();
    addWidget(std::make_unique<Wt::WText>("Jahr wechseln: "));
    auto selectYear = addWidget(std::make_unique<Wt::WComboBox>());
    auto dt = forUser_.get()->debitTimes.find().orderBy("validFrom").limit(1).resultList().front();
    int firstYear = dt->validFrom.year();
    int lastYear = WDate::currentDate().year();
    for(int i=firstYear; i<=lastYear; ++i) selectYear->addItem(std::to_string(i));
    selectYear->setCurrentIndex(year-firstYear);
    selectYear->sactivated().connect([=](WString _year){
      year = std::stoi(_year);
      update();
    });

    // Mitarbeiterauswahl (falls Admin)
    if(user->role == UserRole::Admin) {
      addWidget(std::make_unique<Wt::WText>("Mitarbeiter wechseln: "));
      auto cb = addWidget(std::make_unique<Wt::WComboBox>());

      auto users = session_.session_.find<User>().resultList();
      int idx = 0;
      for(auto u : users) {
        auto loginName = u->authInfos.front()->identity(Auth::Identity::LoginName);
        cb->addItem(loginName);
        if(u == forUser_) cb->setCurrentIndex(idx);
        ++idx;
      }
      cb->sactivated().connect([=](WString name) {
        dbo::Transaction transaction(session_.session_);
        auto list = session_.session_.find<User>().where("name = ?").bind(name).resultList();
        if(list.empty()) return;    /// @todo make error message
        forUser_ = list.front();
        update();
      });
    }

    // Jahresabschluss (falls Admin)
    if(user->role == UserRole::Admin) {
      // Prüfen ob Jahr bereits geschlossen
      if(session_.isYearClosed(year)) {
        addWidget(std::make_unique<Wt::WText>("Jahr "+std::to_string(year)+" bereits abgeschlossen."));
        auto openYear = addWidget(std::make_unique<Wt::WPushButton>(std::to_string(year)+" wieder öffnen"));
        openYear->clicked().connect(this, [=] {
          auto r = Wt::WMessageBox::show("Jahr "+std::to_string(year)+" öffnen",
                                         "Soll das Jahr "+std::to_string(year)+" wirklich wieder geöffnet werden? "
                                         "Alle geänderten Jahresbilanzen für dieses Jahr werden dann zurückgesetzt auf "
                                         "den ursprünglichen Wert. Diese Aktion kann nicht rückgängig gemacht werden.",
                                         StandardButton::Yes | StandardButton::Abort, {AnimationEffect::Fade});
          if(r == StandardButton::Yes) {
            // Jahresabschluss löschen
            {
              dbo::Transaction transaction(session_.session_);
              auto users = session_.session_.find<User>().resultList();
              WDate referenceDate(year,12,31);
              for(auto u : users) {
                u.modify()->annualStatements.find().where("referenceDate = ?").bind(referenceDate).
                    resultList().front().remove();
              }
              session_.cache_isYearClosed[year] = false;
              transaction.commit();
            }
            update();
          }
        });
      }
      // Falls nicht: Jahr ist schließbar, falls vorheriges Jahr bereits geschlossen (oder erstes Jahr überhaupt)
      else if(session_.isYearClosed(year-1) || year == firstYear) {
        // Jahr kann geschlossen werden
        addWidget(std::make_unique<Wt::WText>("Jahr "+std::to_string(year)+" für alle Mitarbeiter abschließen?"));
        auto closeYear = addWidget(std::make_unique<Wt::WPushButton>(std::to_string(year)+" schließen"));
        closeYear->clicked().connect(this, [=] {
          // Jahr abschließen: Für alle Mitarbeiter das Konto zum Jahresende abspeichern
          {
            dbo::Transaction transaction(session_.session_);
            auto users = session_.session_.find<User>().resultList();
            WDate referenceDate(year,12,31);
            for(auto u : users) {
              Wt::Dbo::ptr<AnnualStatement> statement = std::make_unique<AnnualStatement>();
              statement.modify()->referenceDate = referenceDate;
              statement.modify()->balance = u->getBalanceUntil(referenceDate);
              u.modify()->annualStatements.insert(statement);
            }
            session_.cache_isYearClosed[year] = true;
            transaction.commit();
          }
          update();
        });
      }
    }

    // Tabelle mit Jahresübersicht
    auto table = std::make_unique<WTable>();
    table->setHeaderCount(1);
    table->setWidth(WLength("100%"));
    table->addStyleClass("table form-inline table-hover");

    table->elementAt(0, 0)->addWidget(std::make_unique<WText>("Monat"));
    table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Urlaubstage"));
    table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Soll-Stunden"));
    table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Ist-Stunden"));
    table->elementAt(0, 4)->addWidget(std::make_unique<WText>("Monatsbilanz"));
    table->elementAt(0, 5)->addWidget(std::make_unique<WText>("Jahresbilanz"));
    table->elementAt(0, 6)->addWidget(std::make_unique<WText>("Gesamtbilanz"));

    WDate yearBegin(year, 1, 1);
    for(int row=1; row<=12; ++row) {
      WDate first(year, row, 1);
      WDate last = first.addMonths(1).addDays(-1);
      std::string holidays = std::to_string(forUser_->countHolidays(first, last));
      std::string debit = secondsToString(forUser_->getDebitForRange(first, last));
      std::string credit = secondsToString(forUser_->getCreditForRange(first, last));
      std::string balance = secondsToString(forUser_->getBalanceForRange(first, last));
      std::string balanceYear = secondsToString(forUser_->getBalanceForRange(yearBegin, last));
      std::string balanceTotal = secondsToString(forUser_->getBalanceUntil(last, false));

      table->elementAt(row, 0)->addWidget(std::make_unique<WText>(monthNames[row-1]));
      table->elementAt(row, 1)->addWidget(std::make_unique<WText>(holidays));
      table->elementAt(row, 2)->addWidget(std::make_unique<WText>(debit));
      table->elementAt(row, 3)->addWidget(std::make_unique<WText>(credit));
      table->elementAt(row, 4)->addWidget(std::make_unique<WText>(balance));
      table->elementAt(row, 5)->addWidget(std::make_unique<WText>(balanceYear));
      table->elementAt(row, 6)->addWidget(std::make_unique<WText>(balanceTotal));
    }

    WDate yearEnd = yearBegin.addYears(1).addDays(-1);

    std::string holidays = std::to_string(forUser_->countHolidays(yearBegin, yearEnd));
    std::string debit = secondsToString(forUser_->getDebitForRange(yearBegin, yearEnd));
    std::string credit = secondsToString(forUser_->getCreditForRange(yearBegin, yearEnd));
    std::string balance = secondsToString(forUser_->getBalanceForRange(yearBegin, yearEnd));
    std::string balanceYear = secondsToString(forUser_->getBalanceForRange(yearBegin, yearEnd));
    std::string balanceTotal = secondsToString(forUser_->getBalanceUntil(yearEnd));

    table->elementAt(13, 0)->addWidget(std::make_unique<WText>("Summe"));
    table->elementAt(13, 1)->addWidget(std::make_unique<WText>(holidays));
    table->elementAt(13, 2)->addWidget(std::make_unique<WText>(debit));
    table->elementAt(13, 3)->addWidget(std::make_unique<WText>(credit));
    table->elementAt(13, 4)->addWidget(std::make_unique<WText>(balance));
    table->elementAt(13, 5)->addWidget(std::make_unique<WText>(balanceYear));

    // Jahresabschluss editierbar, falls Admin
    if(user->role == UserRole::Admin && session_.isYearClosed(year)) {
      // Prüfen ob Jahr bereits geschlossen
      auto edit = table->elementAt(13, 6)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
      edit->setValue(forUser_->getBalanceUntil(yearEnd)/3600.);
      auto save = table->elementAt(13, 6)->addWidget(std::make_unique<Wt::WPushButton>("OK"));
      save->clicked().connect(this, [=]{
        dbo::Transaction transaction(session_.session_);
        auto statement = forUser_.get()->annualStatements.find().
            where("referenceDate = ?").bind(std::to_string(year)+"-12-31").resultList().front();
        statement.modify()->balance = edit->value()*3600.;
        update();
      });
    }
    else {
      table->elementAt(13, 6)->addWidget(std::make_unique<WText>(balanceTotal));
    }

    addWidget(std::move(table));

}

