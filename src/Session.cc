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

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>
#include <Wt/WException.h>

#include <fstream>
#include <unistd.h>

namespace dbo = Wt::Dbo;

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
    auto sqlite3 = std::make_unique<Dbo::backend::Sqlite3>(WApplication::instance()->appRoot() + "WtTimeTrack.db");
    sqlite3->setProperty("show-queries", "true");
    session_.setConnection(std::move(sqlite3));

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
    try {
      session_.createTables();

      /*
      * Add a default admin/admin account
      */
      registerUser("admin", "admin@example.com", UserRole::Admin, "admin");

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
