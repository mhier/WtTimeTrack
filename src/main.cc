/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#include <unistd.h>
#include <Wt/Date/tz.h>

#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WLocale.h>

#include "WtTimeTrack.h"
#include "Session.h"

using namespace Wt;

static WLocale theLocale;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env) {
    auto app = std::make_unique<WApplication>(env);

    app->setTitle("Zeiterfassung");

    app->root()->addStyleClass("container");
    auto theme = std::make_shared<Wt::WBootstrapTheme>();
    theme->setVersion(Wt::BootstrapVersion::v3);
    theme->setResponsive(true);
    app->setTheme(theme);

    theLocale = WLocale("de_DE");
    theLocale.setTimeZone(date::current_zone());

    app->setLocale(theLocale);

    app->useStyleSheet("WtTimeTrack.css");

    app->root()->addWidget(std::make_unique<WtTimeTrack>());

    return app;
}

int main(int argc, char **argv) {
    try {
      WServer server(argc, argv, WTHTTP_CONFIGURATION);

      server.addEntryPoint(EntryPointType::Application, createApplication, "");

      Session::configureAuth();

      server.run();
    }
    catch (WServer::Exception& e) {
      std::cerr << e.what() << std::endl;
    }
    catch (std::exception &e) {
      std::cerr << "exception: " << e.what() << std::endl;
    }
}
