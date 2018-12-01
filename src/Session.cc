/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Dbo/backend/MySQL.h>

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>
#include <Wt/WException.h>

#include <fstream>
#include <unistd.h>

#include <boost/program_options.hpp>

namespace dbo = Wt::Dbo;
namespace po = boost::program_options;

namespace {

  class MyOAuth : public std::vector<const Auth::OAuthService *> {
    public:
      ~MyOAuth() {
        for (unsigned i = 0; i < size(); ++i) delete (*this)[i];
      }
  };

  Auth::AuthService myAuthService;
  Auth::PasswordService myPasswordService(myAuthService);

} // anon. namespace

void Session::configureAuth() {
    myAuthService.setAuthTokensEnabled(true, "WtTimeTrackCookie");
    myAuthService.setEmailVerificationEnabled(true);

    auto verifier = std::make_unique<Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

    myPasswordService.setVerifier(std::move(verifier));
    myPasswordService.setStrengthValidator(std::make_unique<Auth::PasswordStrengthValidator>());
    myPasswordService.setAttemptThrottlingEnabled(true);
}

Session::Session() {

    po::options_description options("Allowed options");
    options.add_options()
        ("sqlite", "use the sqlite database backend")
        ("sqlite-database", po::value<std::string>(), "file name of the sqlite data base")
        ("mysql", "use the MySQL database backend")
        ("mysql-database", po::value<std::string>(), "MySQL database name")
        ("mysql-user", po::value<std::string>(), "MySQL database user name")
        ("mysql-password", po::value<std::string>(), "MySQL database password")
        ("mysql-host", po::value<std::string>(), "MySQL database host")
        ("fillTestData", "Fill test data to the database for performance testing.")
    ;

    po::variables_map vm;
    store(po::parse_config_file<char>("WtTimeTrack.cfg", options), vm);
    po::notify(vm);

    if(vm.count("sqlite") && vm.count("mysql")) {
      std::cout << "ERROR: cannot use sqlite and mysql at the same time!" << std::endl;
      exit(1);
    }
    if(!vm.count("sqlite") && !vm.count("mysql")) {
      std::cout << "ERROR: You have to specify either sqlite or mysql as a database!" << std::endl;
      exit(1);
    }

    std::unique_ptr<Dbo::SqlConnection> theDB;
    if(vm.count("sqlite")) {
      if(!vm.count("sqlite-database")) {
        std::cout << "ERROR: You have to specify the file name for the sqlite data base (sqlite-database option)" << std::endl;
        exit(1);
      }
      theDB = std::make_unique<Dbo::backend::Sqlite3>( WApplication::instance()->appRoot() +
                                                       vm["sqlite-database"].as<std::string>() );
    }
    else {
      if(!vm.count("mysql-database") || !vm.count("mysql-user") || !vm.count("mysql-password") || !vm.count("mysql-host")) {
        std::cout << "ERROR: Missing parameter for the MySQL data base connection." << std::endl;
        exit(1);
      }
      theDB = std::make_unique<Wt::Dbo::backend::MySQL>( vm["mysql-database"].as<std::string>(),
                                                         vm["mysql-user"].as<std::string>(),
                                                         vm["mysql-password"].as<std::string>(),
                                                         vm["mysql-host"].as<std::string>()      );
    }
    //theDB->setProperty("show-queries", "true");
    session_.setConnection(std::move(theDB));



    session_.mapClass<User>("user");
    session_.mapClass<AuthInfo>("auth_info");
    session_.mapClass<AuthInfo::AuthIdentityType>("auth_identity");
    session_.mapClass<AuthInfo::AuthTokenType>("auth_token");
    session_.mapClass<CreditTime>("creditTime");
    session_.mapClass<DebitTime>("debitTime");
    session_.mapClass<Absence>("absence");
    session_.mapClass<Holiday>("holiday");
    users_ = std::make_unique<UserDatabase>(session_);

    dbo::Transaction transaction(session_);
    bool initialiseDatabase = false;
    try {
      session_.createTables();
      initialiseDatabase = true;
      log("info") << "Database created";
    }
    catch(std::exception& e) {
      log("info") << "Presumably the database already exists, since creating resulted in the following error: " << e.what();
      log("info") << "Using existing database";
    }

    // output SQL for creating tables
    std::ofstream file("createTables.sql");
    file << session_.tableCreationSql();
    file.close();

    if(initialiseDatabase) {

      /*
      * Add a default admin/admin account
      */
      registerUser("admin", "admin@example.com", UserRole::Admin, "admin");

      // fill with test data, if requested in config file
      if(vm.count("fillTestData")) {
        // create 9 additional users
        for(int iuser=0; iuser<9; iuser++) {
          registerUser("test"+std::to_string(iuser), "test@example.com", UserRole::Employee, "test");
        }
        // add data for each user (including admin)
        auto users = session_.find<User>().resultList();
        for(auto user : users) {
          // credit time...
          for(WDate date = WDate(2017,1,1); date != WDate(2018,1,1); date = date.addDays(1)) {
            // 5 entries per day
            for(int entry=0; entry<5; ++entry) {
              Wt::Dbo::ptr<CreditTime> creditTime = std::make_unique<CreditTime>();
              creditTime.modify()->start = WDateTime(date, WTime(9+2*entry,0));
              creditTime.modify()->stop = WDateTime(date, WTime(10+2*entry,55));
              creditTime.modify()->hasClockedOut = true;
              user.modify()->creditTimes.insert(creditTime);
            }
          }
          // debit time... 1 change every month
          for(int debitEntry=0; debitEntry<12; ++debitEntry) {
            Wt::Dbo::ptr<DebitTime> debitTime = std::make_unique<DebitTime>();
            debitTime.modify()->validFrom = WDate(2017, debitEntry+1, 1);
            debitTime.modify()->workHoursPerWeekday = {{8,8,8,8,8,0,0}};
            user.modify()->debitTimes.insert(debitTime);
          }
          // Absences... 1 every month
          for(int vacationEntry=0; vacationEntry<12; ++vacationEntry) {
            Wt::Dbo::ptr<Absence> absence = std::make_unique<Absence>();
            absence.modify()->first = WDate(2017, vacationEntry+1, 10);
            absence.modify()->last = WDate(2017, vacationEntry+1, 15);
            absence.modify()->reason = Absence::Reason::Holiday;
            user.modify()->absences.insert(absence);
          }
        }
        // for all users: holidays... 1 every month
        for(int holidayEntry=0; holidayEntry<12; ++holidayEntry) {
          Wt::Dbo::ptr<Holiday> holiday = std::make_unique<Holiday>();
          holiday.modify()->first = WDate(2017, holidayEntry+1, 10);
          holiday.modify()->last = WDate(2017, holidayEntry+1, 15);
          session_.add(holiday);
        }
      }
    }

    transaction.commit();
}

Session::~Session() {
}

dbo::ptr<User> Session::user() const {
    if (login_.loggedIn()) {
      dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
      dbo::ptr<User> user = authInfo->user();

      if (!user) {
        user = session_.add(Wt::cpp14::make_unique<User>());
        authInfo.modify()->setUser(user);
      }

      return user;
    }
    else {
      return dbo::ptr<User>();
    }
}

std::string Session::userName() const {
    if (login_.loggedIn()) {
      return login_.user().identity(Auth::Identity::LoginName).toUTF8();
    }
    else {
      return std::string();
    }
}

Auth::AbstractUserDatabase& Session::users() {
    return *users_;
}

const Auth::AuthService& Session::auth() {
    return myAuthService;
}

const Auth::AbstractPasswordService& Session::passwordAuth() {
    return myPasswordService;
}


void Session::registerUser(std::string login, std::string email, UserRole role, Wt::WString password) {
    Auth::User newUser = users_->registerNew();
    newUser.addIdentity(Auth::Identity::LoginName, login);
    newUser.setEmail(email);
    myPasswordService.updatePassword(newUser, password);

    // add User data structure and set user role to Admin
    dbo::ptr<AuthInfo> authInfo = users_->find(newUser);
    auto user = session_.add(std::make_unique<User>());
    authInfo.modify()->setUser(user);
    user.modify()->role = role;
    user.modify()->name = login;
    user.flush();
}


void Session::updateUser(Wt::Dbo::ptr<User> user, std::string email, UserRole role, Wt::WString password) {
    // add User data structure and set user role to Admin
    dbo::ptr<AuthInfo> authInfo = user->authInfos.front();
    auto theUser = users_->find(authInfo);
    user.modify()->role = role;
    authInfo.modify()->setEmail(email);
    if(password != "") myPasswordService.updatePassword(theUser, password);
}
