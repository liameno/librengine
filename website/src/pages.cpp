#include "../include/pages.h"

#define DEBUG true //TODO: FALSE

//      TODO: CONFIG        //
#define MAX_RATING          200
#define MIN_RATING          0
//      ============        //

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
#if DEBUG
    logger::print(type, text, ident);
#endif
}

namespace website {
    pages::pages(const config::all &config) {
        this->config = config;

        this->search_ = std::make_shared<search>(config);

        this->center_result_src_format =   "<div class=\"center_result\">"
                                            "<div class=\"content\">"
                                            "<a class=\"title\" href=\"{1}\">{0}<span><i class=\"fa fa-user-secret info_icon info_{6}\"></i></span></a>"
                                            "<div class=\"url\">{1}</div>"
                                            "<div class=\"description\">{2}</div>"
                                            "</div>"
                                            "<div class=\"rating_container\">"
                                            "<div class=\"rating\">"
                                            "<div class=\"counter\">{3}/200</div>"
                                            "<a class=\"plus rating_button\" href=\"{5}/api/plus_rating?id={4}&redirect=1\"><i class=\"fa fa-arrow-up\"></i></a>"
                                            "<a class=\"minus rating_button\" href=\"{5}/api/minus_rating?id={4}&redirect=1\"><i class=\"fa fa-arrow-down\"></i></a>"
                                            "</div>"
                                            "</div>"
                                            "</div>";
    }
    pages::~pages() {

    }

    void pages::init() {
        this->search_->init();
    }

    void pages::set_variables(std::string &page_src) {
        const std::string noscript_src = R"(<noscript><span class="noscript">Encryption doesn't work</span></noscript>)";
        const std::string header_src = R"(<li><a href="/home">Home</a></li><li><a href="/node/info">Node Info</a></li><li><a href="https://github.com/liameno/librengine">Github</a></li>)";

        replace(page_src, "{RSA_PUBLIC_KEY}", search_->rsa_public_key_base64);
        replace(page_src, "{NOSCRIPT_CONTENT}", noscript_src);
        replace(page_src, "{HEADER_CONTENT}", header_src);
    }

    void pages::update(const std::string &id, const std::string &field, const size_t &value) {
        const auto res = config.db_.websites.get(std::stoi(id));
        json result_json = json::parse(res);
        result_json[field] = value;

        config.db_.websites.update(result_json.dump());
    }
    void pages::update(const std::string &id, const std::string &field, const std::string &value) {
        const auto res = config.db_.websites.get(std::stoi(id));
        json result_json = json::parse(res);
        result_json[field] = value;

        config.db_.websites.update(result_json.dump());
    }
    size_t pages::get_number_field_value(const std::string &id, const std::string &field) {
        const auto res = config.db_.websites.get(std::stoi(id));
        json result_json = json::parse(res);
        return result_json[field];
    }
    size_t pages::get_field_count(const std::string &field) {
        const auto res = config.db_.websites.search("*", field);
        json result_json = json::parse(res);
        return result_json["found"];
    }

    void pages::home_p(lambda_args) {
        std::string page_src = config::helper::get_file_content("../frontend/src/index.html");
        const std::string query = req.get_param_value("q");
        replace(page_src, "{QUERY}", query);

        set_variables(page_src);
        res.status = 200;
        res.set_content(page_src, "text/html");
    }
    void pages::search_p(lambda_args) {
        std::string page_src = config::helper::get_file_content("../frontend/src/search.html");
        std::string page_ = req.get_param_value("p");
        std::string is_encryption_enabled_ = req.get_param_value("e");

        std::string query = replace_copy(req.get_param_value("q"), " ", "+");
        bool is_encryption_enabled = is_encryption_enabled_ == "1";
        std::string encryption_key = req.get_param_value("ek");
        size_t page = (!page_.empty()) ? std::stoi(page_) : 1;

        std::string url_params = format("?q={0}&p={1}&e={2}&ek={3}", query, page_, is_encryption_enabled_, encryption_key);
        encryption_key = encryption::base64::easy_decode(encryption_key);

        if (encryption_key.find("END PUBLIC KEY") == -1) {
            res.set_redirect("/", 500);
            return;
        }

        if (is_encryption_enabled) {
            query = search_->rsa.easy_private_decrypt(query);

            if (query.empty()) {
                res.set_redirect("/", 500);
                return;
            }
        }

        auto results = search_->nodes(query, page, is_encryption_enabled);
        auto size = results.size();

        std::string center_results_src;

        for (int i = 0; i < size; ++i) {
            const auto &result = results[i];

            const auto &id = result.id;
            const auto &title = result.title;
            const auto &url = result.url;
            const auto &desc = result.desc;
            const auto &rating = result.rating;
            const auto &has_trackers = result.has_trackers ? "bad" : "good";
            const auto &node_url = result.node_url;

            std::string result_src = format(center_result_src_format, title, url, desc, rating, id, node_url, has_trackers);
            center_results_src.append(result_src);
        }

        if (center_results_src.empty()) center_results_src = "Not Found";

        if (is_encryption_enabled) {
            encryption::rsa req_rsa;
            req_rsa.easy_read_public_key_buffer(encryption_key);
            center_results_src = req_rsa.easy_public_encrypt(center_results_src);

            if (center_results_src.empty()) {
                res.set_redirect("/", 500);
                return;
            }
        }

        std::string url = req.path + url_params;

        replace(page_src, "{CENTER_RESULTS}", center_results_src);
        replace(page_src, "{QUERY}", query);
        replace(page_src, "{PREV_PAGE}", replace_copy(url, "&p=" + page_, "&p=" + std::to_string((page > 1) ? page - 1 : 1)));
        replace(page_src, "{NEXT_PAGE}", replace_copy(url, "&p=" + page_, "&p=" + std::to_string(page + 1)));

        set_variables(page_src);
        res.status = 200;
        res.set_content(page_src, "text/html");
    }
    void pages::node_info_p(lambda_args) {
        std::string page_src = config::helper::get_file_content("../frontend/src/node/info.html");
        replace(page_src, "{PAGES_COUNT}", std::to_string(get_field_count("url")));

        set_variables(page_src);
        res.status = 200;
        res.set_content(page_src, "text/html");
    }
    void pages::api_get_rsa_public_key(lambda_args) {
        res.status = 200;
        res.set_content(search_->rsa_public_key, "text/html");
    }
    void pages::api_plus_rating(lambda_args) {
        const std::string id = req.get_param_value("id");
        const std::string is_redirect = req.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating");

        if (rating < MAX_RATING) {
            update(id, "rating", rating + 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            const auto referer = req.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            res.set_redirect(redirect_url, 301);
        } else {
            res.status = 200;
        }
    }
    void pages::api_minus_rating(lambda_args) {
        const std::string id = req.get_param_value("id");
        const std::string is_redirect = req.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating");

        if (rating > MIN_RATING) {
            update(id, "rating", rating - 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            res.status = 301;
            const auto referer = req.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            res.set_redirect(redirect_url);
        } else {
            res.status = 200;
        }
    }
    void pages::api_search(lambda_args) {
        std::string query = replace_copy(req.get_param_value("q"), " ", "+");
        std::string page_ = req.get_param_value("p");
        std::string is_encryption_enabled_ = req.get_param_value("e");

        bool is_encryption_enabled = is_encryption_enabled_ == "1";
        std::string encryption_key = req.get_param_value("ek");
        size_t page = (!page_.empty()) ? std::stoi(page_) : 1;

        json page_src;

        if_debug_print(logger::type::info, "query = " + query, req.path);

        if (is_encryption_enabled) {
            encryption_key = encryption::base64::easy_decode(encryption_key);
            query = search_->rsa.easy_private_decrypt(query);

            if (query.empty()) {
                res.set_redirect("/", 500);
                return;
            }

            if_debug_print(logger::type::info, "decrypted query = " + query, req.path);
        }

        const auto search_results = search_->local(query, page);
        auto sr_size = search_results.size();
        if_debug_print(logger::type::info, "found = " + std::to_string(sr_size), req.path);

        for (int i = 0; i < sr_size; ++i) {
            const auto &result = search_results[i];

            page_src["results"][i]["title"] = result.title;
            page_src["results"][i]["url"] = result.url;
            page_src["results"][i]["desc"] = result.desc;
            page_src["results"][i]["rating"] = result.rating;
            page_src["results"][i]["id"] = result.id;
            page_src["results"][i]["has_trackers"] = result.has_trackers;
        }

        page_src["count"] = search_results.size();
        std::string result;

        try {
            result = page_src.dump(-1, ' ', false, json::error_handler_t::replace);
        } catch (const std::exception& e) {
            if_debug_print(logger::type::error, e.what(), req.path);
            res.set_redirect("/", 500);
            return;
        }

        if (is_encryption_enabled) {
            encryption::rsa req_rsa;
            req_rsa.easy_read_public_key_buffer(encryption_key);
            result = req_rsa.easy_public_encrypt(result);

            if (result.empty()) {
                res.set_redirect("/", 500);
                return;
            }
        }

        res.status = 200;
        res.set_content(result, "application/json");
    }
    void pages::api_node_info(lambda_args) {
        json page_src;
        page_src["pages_count"] = get_field_count("url");

        res.status = 200;
        res.set_content(page_src.dump(), "application/json");
    }
    void pages::api_node_info_chart(lambda_args) {
        json page_src;
        auto now = time(nullptr);
        auto min_gm = gmtime(&now);

        for (int i = 1; i < 31; ++i) {
            min_gm->tm_mday = i;
            min_gm->tm_hour = 0;
            min_gm->tm_min = 0;
            min_gm->tm_sec = 0;

            auto min = timegm(min_gm);

            min_gm->tm_mday = i;
            min_gm->tm_hour = 23;
            min_gm->tm_min = 59;
            min_gm->tm_sec = 59;

            auto max = timegm(min_gm);

            auto filter_by = "date:>" + std::to_string(min) +  " && date:<" + std::to_string(max);
            filter_by = http::url::escape(filter_by);

            auto found = config.db_.websites.search("*", "url", {{"filter_by", filter_by}, {"num_typos", "0"}});
            json json = json::parse(found);
            auto found_count = json["found"];

            page_src["data"][std::to_string(i)] = found_count;
        }

        res.status = 200;
        res.set_content(page_src.dump(), "application/json");
    }
    void pages::not_found(lambda_args) {
        res.status = 301;
        res.set_redirect("/home");
    }
}