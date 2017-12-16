/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordStrengthValidator.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/GoogleService.h"
#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>
#include <Wt/WException.h>

#ifndef WT_WIN32
#include <unistd.h>
#endif

#if !defined(WT_WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#endif

namespace dbo = Wt::Dbo;

namespace {

#ifdef HAVE_CRYPT
  class UnixCryptHashFunction : public Auth::HashFunction {
    public:
      virtual std::string compute(const std::string& msg, const std::string& salt) const {
        std::string md5Salt = "$1$" + salt;
        return crypt(msg.c_str(), md5Salt.c_str());
      }

      virtual bool verify(const std::string& msg, const std::string& salt, const std::string& hash) const {
        return crypt(msg.c_str(), hash.c_str()) == hash;
      }

      virtual std::string name () const {
        return "crypt";
      }
  };
#endif // HAVE_CRYPT

  class MyOAuth : public std::vector<const Auth::OAuthService *> {
    public:
      ~MyOAuth() {
        for (unsigned i = 0; i < size(); ++i) delete (*this)[i];
      }
  };

  Auth::AuthService myAuthService;
  Auth::PasswordService myPasswordService(myAuthService);
  MyOAuth myOAuthServices;

} // anon. namespace

void Session::configureAuth() {
    myAuthService.setAuthTokensEnabled(true, "hangmancookie");
    myAuthService.setEmailVerificationEnabled(true);

    auto verifier = std::make_unique<Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

  #ifdef HAVE_CRYPT
    // We want to still support users registered in the pre - Wt::Auth
    // version of the hangman example
    verifier->addHashFunction(std::make_unique<UnixCryptHashFunction>());
  #endif

    myPasswordService.setVerifier(std::move(verifier));
    myPasswordService.setStrengthValidator(std::make_unique<Auth::PasswordStrengthValidator>());
    myPasswordService.setAttemptThrottlingEnabled(true);

    if (Auth::GoogleService::configured())
      myOAuthServices.push_back(new Auth::GoogleService(myAuthService));
}

Session::Session() {
    auto sqlite3 = std::make_unique<Dbo::backend::Sqlite3>(WApplication::instance()->appRoot() + "zeiterfassung.db");
    sqlite3->setProperty("show-queries", "true");
    session_.setConnection(std::move(sqlite3));

    session_.mapClass<User>("user");
    session_.mapClass<AuthInfo>("auth_info");
    session_.mapClass<AuthInfo::AuthIdentityType>("auth_identity");
    session_.mapClass<AuthInfo::AuthTokenType>("auth_token");
    session_.mapClass<CreditTime>("creditTime");
    session_.mapClass<DebitTime>("debitTime");
    session_.mapClass<Absence>("absence");
    users_ = std::make_unique<UserDatabase>(session_);

    dbo::Transaction transaction(session_);
    try {
      session_.createTables();

      /*
      * Add a default admin/admin account
      */
      Auth::User adminUser = users_->registerNew();
      adminUser.addIdentity(Auth::Identity::LoginName, "admin");
      myPasswordService.updatePassword(adminUser, "admin");

      // add User data structure and set user role to Admin
      dbo::ptr<AuthInfo> authInfo = users_->find(adminUser);
      auto user = session_.add(std::make_unique<User>());
      authInfo.modify()->setUser(user);
      user.modify()->role = UserRole::Admin;
      user.flush();

      log("info") << "Database created";
    }
    catch(std::exception& e) {
      log("info") << "Presumably the database already exists, since creating resulted in the following error: " << e.what();
      log("info") << "Using existing database";
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

const std::vector<const Auth::OAuthService *>& Session::oAuth() {
    return myOAuthServices;
}
