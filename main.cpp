#include <iostream>
#include <map>
#include <string>
#include<bits/stdc++.h>
#include "httplib.h"

const char* REGISTER_PAGE = R"REGISTER_PAGE(

<form method="POST">
    <label for="login">Login:</label>
    <input name="login" />

    <label for="password">Password:</label>
    <input name="password" type="password" />

    <input type="submit" />
</form>

)REGISTER_PAGE";

const char* LOGIN_PAGE = R"LOGIN_PAGE(

<form method="POST">
    <label for="login">Login:</label>
    <input name="login" />

    <label for="password">Password:</label>
    <input name="password" type="password" />

    <input type="submit" />
</form>

)LOGIN_PAGE";




int main()
{
    // объявили переменную сервер
    httplib::Server svr;
    std::vector<std::pair<std::string, std::string>> known_users = {};
    bool logged = 0;

    svr.Get("/stop", [&](const httplib::Request & /*unused*/,
                       httplib::Response & /*unused*/) { svr.stop(); });


    // Сервер должен запросить имя пользователя и пароль и, если эти данные известны системе,
    // вывести приветствие.
    svr.Get("/login", [](const httplib::Request &req, httplib::Response &res) {


        for (auto value : req.headers)
            std::cout << value.first << ": " << value.second << std::endl;

        res.set_content(LOGIN_PAGE, "text/html");
    });

    svr.Post("/login", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "Thinking of the ligging" << std::endl;
        httplib::Params params;
        httplib::detail::parse_query_text(req.body, params);
        bool found = false;
        std::pair<std::string, std::string> usr;
        for (auto p : params) {
            std::cout << p.first << " = " << p.second << std::endl;

            if(p.first == "login"){
              usr.first = p.second;
            }
            else{
              usr.second = p.second;
            }

        }
        for(auto s : known_users){
          if(s == usr){
            std::cout << "I will try to write page for that\n";
            logged = true;
            found = true;
            break;
          }
        }
        if(!found){
          std::cout << "Nah, I don't think so\n";
        }

    });


    // Завершить работу пользователя
    svr.Get("/logout", [&](const httplib::Request &, httplib::Response &res) {
      logged = 0;
    });

    // Вывести форму регистрации
    svr.Get("/register", [](const httplib::Request &, httplib::Response &res) {
        std::cout << "In GET handler" << std::endl;
        res.set_content(REGISTER_PAGE, "text/html");
    });

    // Обработать отправку формы регистрации
    svr.Post("/register", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "In POST handler" << std::endl;
        httplib::Params params;
        httplib::detail::parse_query_text(req.body, params);
        std::pair<std::string, std::string> usr;
        for (auto p : params) {
            std::cout << p.first << " = " << p.second << std::endl;
            if(p.first == "login"){
              usr.first = p.second;
            }
            else{
              usr.second = p.second;
            }
        }
        known_users.push_back(usr);
    });

    // запуск сервера:
    // "слушает" (ожидает запросы) на порту с номером 8080
    // адрес 0.0.0.0 — на всех доступных IP-адресах
    svr.listen("0.0.0.0", 8080);
}
