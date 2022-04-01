#ifndef HTTP_H
#define HTTP_H

#include <iostream>
#include <memory>
#include <vector>
#include <curl/curl.h>
#include <memory>
#include <optional>
#include <csetjmp>
#include <csignal>
#include <cstdlib>

namespace librengine::http {
    size_t write_function(void *ptr, size_t size, size_t nmemb, std::string *data);

    enum class proxy_type {
        http,
        https,
        socks4,
        socks5,
    };

    struct proxy {
        std::string ip;
        std::string port;
        std::string full;
        proxy_type type;

        void set_full(const std::string &full = {});
        std::string compute_curl_format() const;

        proxy(const std::string &ip, const std::string &port, const proxy_type &type = proxy_type::http);
        explicit proxy(const std::string &full);
        explicit proxy(const std::string &full, const proxy_type &type);
    };

    struct header {
        std::string name;
        std::string value;
        std::string full;

        void set_full(const std::string &full = {});

        explicit header(const std::string &full);
        explicit header(const std::string &name, const std::string &value);
    };

    class url {
    public:
        std::string text;
        std::optional<std::string> scheme;
        std::optional<std::string> user;
        std::optional<std::string> password;
        std::optional<std::string> options;
        std::optional<std::string> host;
        std::optional<std::string> zone_id;
        std::optional<std::string> port;
        std::optional<std::string> path;
        std::optional<std::string> query;
        std::optional<std::string> fragment;
    private:
        Curl_URL *current_curl_url;
    public:
        explicit url(std::string text);
        ~url();

        void parse();
        void compute_text();

        void set(const CURLUPart &what, const std::string &value);

        bool is_localhost();
    };

    class request {
    public:
        struct result_s {
            std::optional<std::string> response;
            long code {0};
            CURLcode curl_code; //https://curl.se/libcurl/c/libcurl-errors.html
        };
        struct {
            int timeout_s {5};
            std::optional<std::string> user_agent;
            std::optional<http::proxy> proxy;
            std::shared_ptr<std::vector<header>> headers;
        } options;
        result_s result;
    private:
        CURL *curl;
        std::string url;
        std::string data;
        std::string type;

        static curl_slist *headers_to_curl_struct(const std::shared_ptr<std::vector<header>> &headers);
    public:
        explicit request(std::string url, const std::optional<std::string> &data = std::nullopt, const std::optional<std::string> &type = std::nullopt, const bool &is_set_secure_headers = true);
        ~request();

        void perform();
    };
}

#endif