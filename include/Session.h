/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <vector>

#include <Wt/Auth/Login.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "User.h"
#include "Holiday.h"

using namespace Wt;

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session {
  public:
    static void configureAuth();

    Session();
    ~Session();

    Auth::AbstractUserDatabase& users();
    Auth::Login& login() { return login_; }

    /*
    * These methods deal with the currently logged in user
    */
    std::string userName() const;

    Dbo::ptr<User> user() const;

    static const Auth::AuthService& auth();
    static const Auth::AbstractPasswordService& passwordAuth();

    mutable Dbo::Session session_;

    // add a new user
    void registerUser(std::string login, std::string email, UserRole role, Wt::WString password);

    // update a user (password will not be changed if empty)
    void updateUser(Wt::Dbo::ptr<User> user, std::string email, UserRole role, Wt::WString password);


  private:
    std::unique_ptr<UserDatabase> users_;
    Auth::Login login_;
};

#endif //SESSION_H_
