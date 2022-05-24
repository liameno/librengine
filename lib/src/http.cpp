#include "http.h"

#include <utility>

#include "str.h"

namespace librengine::http {
    size_t write_function(void *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append((char *) ptr, size * nmemb);
        return size * nmemb;
    }
    void proxy::set_full(const std::string &full) {
        if (full.empty()) {
            this->full = ip + ":" + port;
        } else {
            this->full = full;
        }
    }

    proxy::proxy(const std::string &ip, const std::string &port, const http::proxy_type &type) {
        this->ip = ip;
        this->port = port;
        this->type = type;
        set_full();
    }
    proxy::proxy(const std::string &full) {
        set_full(full);
    }
    proxy::proxy(const std::string &full, const proxy_type &type) {
        auto split = str::split(full, ":");

        if (split.size() > 1) {
            this->ip = split[0];
            this->port = split[1];
        }

        this->type = type;
        set_full(full);
    }

    std::string proxy::compute_curl_format() const {
        std::string result;

        if (full.empty()) return {};
        if (str::starts_with(full, "http") || str::starts_with(full, "socks")) return full;

        switch (type) {
            case proxy_type::http:
                result.append("http");
                break;
            case proxy_type::https:
                result.append("https");
                break;
            case proxy_type::socks4:
                result.append("socks4");
                break;
            case proxy_type::socks5:
                result.append("socks5");
                break;
        }

        result.append("://");
        result.append(full);
        return result;
    }
    void header::set_full(const std::string &full) {
        if (full.empty()) {
            this->full.clear();
            this->full.append(this->name);
            this->full.append(": ");
            this->full.append(this->value);
        } else {
            const auto splited = str::split(full, ":");

            if (splited.size() > 1) {
                this->name = splited[0];
                this->value = splited[1];
                str::remove_first_char(this->value);
            }

            this->full = full;
        }
    }

    header::header(const std::string &full) {
        set_full(full);
    }
    header::header(const std::string &name, const std::string &value) {
        this->name = name;
        this->value = value;
        set_full();
    }

    url::url(std::string text) : text(std::move(text)) {
        this->current_curl_url = curl_url();
    }
    url::~url() {
        curl_url_cleanup(current_curl_url);
    }

    std::string url::escape(const std::string &s) {
        CURL *curl = curl_easy_init();
        char *output = curl_easy_escape(curl, s.c_str(), s.size());
        std::string result = output;

        curl_free(output);
        curl_easy_cleanup(curl);

        return result;
    }

    void url::parse() {
        //https://curl.se/libcurl/c/parseurl.html
        curl_url_set(current_curl_url, CURLUPART_URL, text.c_str(), 0/*CURLU_DEFAULT_SCHEME*/);

        char *scheme;
        char *user;
        char *password;
        char *options;
        char *host;
        char *zone_id;
        char *path;
        char *query;
        char *fragment;

        auto c = curl_url_get(current_curl_url, CURLUPART_SCHEME, &scheme, 0);
        if (!c) {
            this->scheme = scheme;
            curl_free(scheme);
        }

        c = curl_url_get(current_curl_url, CURLUPART_USER, &user, 0);
        if (!c) {
            this->user = user;
            curl_free(user);
        }

        c = curl_url_get(current_curl_url, CURLUPART_PASSWORD, &password, 0);
        if (!c) {
            this->password = password;
            curl_free(password);
        }

        c = curl_url_get(current_curl_url, CURLUPART_OPTIONS, &options, 0);
        if (!c) {
            this->options = options;
            curl_free(options);
        }

        c = curl_url_get(current_curl_url, CURLUPART_HOST, &host, 0);
        if (!c) {
            this->host = host;
            curl_free(host);
        }

        c = curl_url_get(current_curl_url, CURLUPART_ZONEID, &zone_id, 0);
        if (!c) {
            this->zone_id = zone_id;
            curl_free(zone_id);
        }

        c = curl_url_get(current_curl_url, CURLUPART_PATH, &path, 0);
        if (!c) {
            this->path = path;
            curl_free(path);
        }

        c = curl_url_get(current_curl_url, CURLUPART_QUERY, &query, 0);
        if (!c) {
            this->query = query;
            curl_free(query);
        }

        c = curl_url_get(current_curl_url, CURLUPART_FRAGMENT, &fragment, 0);
        if (!c) {
            this->fragment = fragment;
            curl_free(fragment);
        }

        compute_text();
    }
    void url::compute_text() {
        char *url;
        auto c = curl_url_get(current_curl_url, CURLUPART_URL, &url, 0);

        if (!c) {
            this->text = url;

            if (str::get_last_char(this->text) == '#') {
                str::remove_last_char(this->text);
            }

            curl_free(url);
        }
    }

    void url::set(const CURLUPart &what, const std::string &value) {
        curl_url_set(current_curl_url, what, value.c_str(), 0);
    }

    bool url::is_localhost() {
        std::string h = host.value_or("");
        return h == "127.0.0.1" || h == "0.0.0.0" || "localhost";
    }

    request::request(std::string url, const std::optional<std::string> &data, const std::optional<std::string> &type, const bool &is_set_secure_headers)
    :url(std::move(url)) {
        this->data = data.value_or("");
        this->type = type.value_or("GET");
        this->curl = curl_easy_init();
        this->options.headers = std::make_shared<std::vector<header>>();

        this->url = str::replace(this->url, " ", "%20");

        if (is_set_secure_headers) {
            this->options.headers->emplace_back("DNT", "1"); //don't track
            this->options.headers->emplace_back("Sec-GPC", "1"); //don't sell or share
            this->options.headers->emplace_back("Upgrade-Insecure-Requests","1"); //can redirect to a secure version
        }
    }
    request::~request() {
        curl_easy_cleanup(curl);
    }

    curl_slist *request::headers_to_curl_struct(const std::shared_ptr<std::vector<header>> &headers) {
        if (headers->empty()) return nullptr;
        struct curl_slist *curl_headers = nullptr;

        for (auto &header_: *headers) {
            if (header_.value.empty()) continue;
            curl_headers = curl_slist_append(curl_headers, header_.full.c_str());
        }

        if (curl_headers == nullptr) { curl_headers = curl_slist_append(curl_headers, ""); }
        return curl_headers;
    }

    void request::perform() {
        if (options.proxy) { curl_easy_setopt(curl, CURLOPT_PROXY, options.proxy.value().compute_curl_format().c_str()); }
        if (options.user_agent) { curl_easy_setopt(curl, CURLOPT_USERAGENT, options.user_agent.value().c_str()); }
        if (options.timeout_s > 0) { curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeout_s); }

        struct curl_slist *curl_headers = headers_to_curl_struct(options.headers);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
        std::string temp_response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, type.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &temp_response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_function);

        result.curl_code = curl_easy_perform(curl);
        if (!temp_response.empty()) result.response = temp_response;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.code);
        curl_slist_free_all(curl_headers);
    }
}