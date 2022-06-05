#include "httplib.h"
#include <bits/stdc++.h>
#include <iostream>
#include <map>
#include <string>

int main(int /*unused*/, char **argv)
{
    httplib::Client cli("0.0.0.0:8080");

    // auto res = cli.Post("/register", {"login","password"}, "text/plain");

    cli.set_basic_auth(argv[1], argv[2]);

    auto res = cli.Get("/login");
    std::cout << res->body << std::endl;
    if (res->body == "Not logged.") {
        httplib::Params params;
        params.emplace("login", argv[1]);
        params.emplace("password", argv[2]);

        res = cli.Post("/register", params);
        std::cout << res->body << std::endl;
        res = cli.Get("/login");

        std::cout << res->body << std::endl;
    }

    res = cli.Get("/logout");
    std::cout << res->body << std::endl;
    cli.Get("/stop");

    return 0;
}
