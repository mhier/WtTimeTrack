/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include "UserDialog.h"

#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include <Wt/WPushButton.h>
#include <Wt/WGridLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WComboBox.h>

UserDialog::UserDialog(Updateable *owner, Session &session, Wt::Dbo::ptr<User> user)
: Wt::WDialog("Benutzer"), owner_(owner), session_(session), user_(user)
{
    contents()->addStyleClass("form-group");

    Dbo::Transaction transaction(session_.session_);
    if(session_.user()->role != UserRole::Admin) return;

    bool createNew = false;
    if(user_.get() == nullptr) createNew = true;

    auto grid = contents()->setLayout(std::make_unique<Wt::WGridLayout>());

    grid->addWidget(std::make_unique<Wt::WText>("Login: "), 0, 0);
    auto editLogin = grid->addWidget(std::make_unique<Wt::WLineEdit>(), 0, 1);

    grid->addWidget(std::make_unique<Wt::WText>("E-Mail: "), 1, 0);
    auto editEmail = grid->addWidget(std::make_unique<Wt::WLineEdit>(), 1, 1);

    grid->addWidget(std::make_unique<Wt::WText>("Rolle: "), 2, 0);
    auto editRole = grid->addWidget(std::make_unique<Wt::WComboBox>(), 2, 1);
    editRole->addItem("Mitarbeiter");
    editRole->addItem("Admin");

    if(createNew) {
      grid->addWidget(std::make_unique<Wt::WText>("Anfangs-Passwort: "), 3, 0);
    }
    else {
      grid->addWidget(std::make_unique<Wt::WText>("Passwort überschreiben (freilassen zum Beibehalten): "), 3, 0);
    }
    auto editPassword = grid->addWidget(std::make_unique<Wt::WLineEdit>(), 3, 1);

    if(!createNew) {
      editLogin->setDisabled(true);
      editLogin->setText(user_->authInfos.front()->identity(Wt::Auth::Identity::LoginName));
      editEmail->setText(user_->authInfos.front()->email());
      editRole->setCurrentIndex(static_cast<int>(user_->role));
    }

    contents()->setWidth(600);

    if(!createNew && user_ != session_.user()) {   // existing user might be deleted - unless it is us!
      Wt::WPushButton *del = footer()->addWidget(std::make_unique<Wt::WPushButton>("Löschen"));
      del->clicked().connect(this, [=] {
        dbo::Transaction transaction(session_.session_);
        user_.remove();
        owner_->update();
        hide();
      } );
    }

    Wt::WPushButton *ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Ok"));
    ok->setDefault(true);
    ok->clicked().connect(this, [=] {
      dbo::Transaction transaction(session_.session_);
      if(createNew) {   // create new on ok
        session_.registerUser( editLogin->text().toUTF8(), editEmail->text().toUTF8(),
                               static_cast<UserRole>(editRole->currentIndex()), editPassword->text() );
      }
      else {
        session_.updateUser( user_, editEmail->text().toUTF8(),
                               static_cast<UserRole>(editRole->currentIndex()), editPassword->text() );
      }
      owner_->update();
      hide();
    } );

    Wt::WPushButton *cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Abbrechen"));
    cancel->clicked().connect(this, [=] {hide();} );

}

