#include "httplib.h"
#include <bits/stdc++.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
typedef unsigned char BYTE;

const char *REGISTER_PAGE = R"REGISTER_PAGE(

<form method="POST">
    <label for="login">Login:</label>
    <input name="login" />

    <label for="password">Password:</label>
    <input name="password" type="password" />

    <input type="submit" />
</form>

)REGISTER_PAGE";

static inline bool is_base64(BYTE c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<BYTE> base64_decode(std::string const &encoded_string)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "0123456789+/";
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    BYTE char_array_4[4], char_array_3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encoded_string[in_] != '=') &&
           is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}

int main()
{
    // объявили переменную сервер
    httplib::Server svr;
    std::vector<std::pair<std::string, std::string>> known_users = {};
    bool logged = false;

    svr.Get("/stop", [&](const httplib::Request & /*unused*/,
                         httplib::Response & /*unused*/) { svr.stop(); });

    // Сервер должен запросить имя пользователя и пароль и, если эти данные
    // известны системе, вывести приветствие.

    svr.Get("/login", [&](const httplib::Request &req, httplib::Response &res) {
        // std::cout << req.remote_port << " " << res.status << std::endl;
        std::string s = "";
        for (auto value : req.headers) {
            // std::cout << value.first << " " <<value.second << std::endl;
            if (value.first == "Authorization") {
                s = value.second;
            }
        }
        if (s != "") {
            if (!logged) {
                s = s.erase(0, 6);
                // std::cout << s << std::endl;
                std::vector<BYTE> data1 = base64_decode(s);
                // std::cout << data1.data() << std::endl;
                std::string data(reinterpret_cast<const char *>(data1.data()));
                // std::cout << data << std::endl;
                int pos = data.find(":");
                std::string login = data.substr(0, pos);
                std::string password = data.erase(0, pos + 1);
                std::cout << login << " " << password << std::endl;
                // std::cout << "stuff\n";

                for (auto p : known_users) {
                    if (p.first == login && p.second == password) {
                        std::cout << "logged\n";
                        res.status = 200;
                        logged = true;
                        res.set_content("You have been logged! :-)",
                                        "text/plain");
                        break;
                    }
                }
                if (!logged) {
                    res.set_content("Not logged.", "text/plain");
                }

            } else {
                res.set_content("Already lgged.", "text/plain");
            }
        } else {

            res.status = 401;
            res.set_header("WWW-Authenticate",
                           "Basic realm=\"Enter username\", charset=\"UTF-8\"");
        }
    });

    // Завершить работу пользователя
    svr.Get("/logout", [&](const httplib::Request &, httplib::Response &res) {
        //  std::cout << res.status << std::endl;
        res.set_content("Logged out.", "text/plain");
        logged = false;
        res.status = 401;
    });

    // Вывести форму регистрации
    svr.Get("/register", [](const httplib::Request &, httplib::Response &res) {
        //  std::cout << res.status << std::endl;

        std::cout << "In GET handler" << std::endl;
        res.set_content(REGISTER_PAGE, "text/html");
    });

    // Обработать отправку формы регистрации
    svr.Post("/register",
             [&](const httplib::Request &req, httplib::Response &res) {
                 // std::cout << res.status << std::endl;

                 std::cout << "In POST handler" << std::endl;
                 httplib::Params params;
                 httplib::detail::parse_query_text(req.body, params);
                 std::pair<std::string, std::string> usr;
                 for (auto p : params) {
                     std::cout << p.first << " = " << p.second << std::endl;
                     if (p.first == "login") {
                         usr.first = p.second;
                     } else {
                         usr.second = p.second;
                     }
                 }
                 known_users.push_back(usr);
                 res.set_content("You've done great!", "text/plain");
             });

    // запуск сервера:
    // "слушает" (ожидает запросы) на порту с номером 8080
    // адрес 0.0.0.0 — на всех доступных IP-адресах
    svr.listen("0.0.0.0", 8080);
}
