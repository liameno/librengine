#include "../include/pages.h"

#define DEBUG true //TODO: FALSE

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
#if DEBUG
    logger::print(type, text, ident);
#endif
}

namespace website {
    pages::pages(const config::all &config) {
        this->config = config;
        this->rsa = encryption::rsa();
        this->rsa.generate_keys(1024);
    }
    void pages::init() {
        for (const auto &node : config.global_.nodes) {
            http::request request_(node.url + "/api/get_rsa_public_key");
            if (!http::url(node.url).is_localhost()) request_.options.proxy = config.website_.proxy;
            request_.perform();

            rsa_public_keys.insert({node.url, request_.result.response.value_or("")});
        }
    }

    void pages::set_variables(std::string &page_src) {
        const std::string noscript_src = R"(<noscript><span class="noscript">Encryption doesn't work without js</span></noscript>)";
        const std::string header_src = R"(<li><a href="/home">Home</a></li><li><a href="/node/info">Node Info</a></li><li><a href="https://github.com/liameno/librengine">Github</a></li>)";

        auto key = rsa.get_public_key_buffer();

        str::replace_ref(page_src, "{RSA_PUBLIC_KEY}", encryption::base64::easy_encode(key));
        str::replace_ref(page_src, "{NOSCRIPT_CONTENT}", noscript_src);
        str::replace_ref(page_src, "{HEADER_CONTENT}", header_src);
    }

    void pages::update(const std::string &id, const std::string &field, const size_t &value) {
        const auto response = config.db_.websites.get(std::stoi(id));
        nlohmann::json result_json = nlohmann::json::parse(response);
        result_json[field] = value;

        config.db_.websites.update(result_json.dump());
    }
    void pages::update(const std::string &id, const std::string &field, const std::string &value) {
        const auto response = config.db_.websites.get(std::stoi(id));
        nlohmann::json result_json = nlohmann::json::parse(response);
        result_json[field] = value;

        config.db_.websites.update(result_json.dump());
    }
    size_t pages::get_number_field_value(const std::string &id, const std::string &field) {
        const auto response = config.db_.websites.get(std::stoi(id));
        nlohmann::json result_json = nlohmann::json::parse(response);
        return result_json[field];
    }
    std::optional<std::vector<librengine::search_result>> pages::search(const std::string &q, const size_t &p) {
        const auto response = config.db_.websites.search(q, "url,title,desc", {{"page", std::to_string(p)}});
        nlohmann::json result_json = nlohmann::json::parse(response);

        const auto body = result_json["hits"];
        if (body.is_null() || body.empty()) return std::nullopt;

        size_t value = body.size();

        std::vector<search_result> results;
        results.reserve(value);

        for (int i = 0; i < value; ++i) {
            search_result result;
            auto hit = body[i];
            auto hit_doc = hit["document"];

            try {
                result.id = hit_doc["id"];
                result.title = hit_doc["title"];
                result.url = hit_doc["url"];
                result.desc = hit_doc["desc"];
                result.rating = hit_doc["rating"];
                result.has_ads = hit_doc["has_ads"];
                result.has_analytics = hit_doc["has_analytics"];
            } catch (const nlohmann::json::exception &e) {
                continue;
            }

            results.push_back(result);
        }

        return results;
    }
    size_t pages::get_field_count(const std::string &field) {
        const auto response = config.db_.websites.search("*", field);
        nlohmann::json result_json = nlohmann::json::parse(response);
        return result_json["found"];
    }

    void pages::home(const Request &request, Response &response) {
        std::string page_src = config::helper::get_file_content("../frontend/src/index.html");
        const std::string query = request.get_param_value("q");
        str::replace_ref(page_src, "{QUERY}", query);

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void pages::search(const Request &request, Response &response) {
        std::string page_src = config::helper::get_file_content("../frontend/src/search.html");
        std::string query = str::replace(request.get_param_value("q"), " ", "+");
        std::string e_ = request.get_param_value("e");
        std::string ek_ = request.get_param_value("ek");
        const std::string p_ = request.get_param_value("p");
        const size_t page = (!p_.empty()) ? std::stoi(p_) : 1;
        const std::string center_result_src_format = "<div class=\"center_result\">"
                                                     "<div class=\"content\">"
                                                     "<a class=\"title\" href=\"{1}\">{0}<span><i class=\"fa fa-ad info_icon info_{6}\"></i><i class=\"fa fa-user-secret info_icon info_{7}\"></i></span></a>"
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

        std::string params_s = str::format("?q={0}&p={1}&e={2}&ek={3}", query, p_, e_, ek_);
        ek_ = encryption::base64::easy_decode(ek_);

        if_debug_print(logger::type::info, "query = " + query, request.path);

        if (e_ == "1") {
            query = rsa.easy_private_decrypt(query);

            if (query.empty()) {
                if_debug_print(logger::type::error, rsa.get_last_error(), request.path);
                return;
            }

            if_debug_print(logger::type::info, "decrypted query = " + query, request.path);
        }

        std::string center_results_src;

        for (const auto &node : config.global_.nodes) {
            if_debug_print(logger::type::info, "node = " + node.url, request.path);
            std::string params_s2;

            if (e_ == "1") {
                encryption::rsa rsa_node;
                auto key = rsa_public_keys.find(node.url)->second;
                rsa_node.read_public_key_buffer(key.data(), key.size());
                auto encrypted_base64 = rsa_node.easy_public_encrypt(query);

                if (encrypted_base64.empty()) {
                    if_debug_print(logger::type::error, rsa_node.get_last_error(), request.path);
                    return;
                }

                auto public_key = rsa.get_public_key_buffer();
                auto key2 = encryption::base64::easy_encode(public_key); //error of curl (CURLE_URL_MALFORMAT)
                params_s2 = str::format("?q={0}&p={1}&e=1&ek={2}", encrypted_base64, p_, key2);
            }

            http::request request_(node.url + "/api/search" + params_s2);
            if (!http::url(node.url).is_localhost()) request_.options.proxy = config.website_.proxy;
            request_.perform();

            if_debug_print(logger::type::info, "search result code = " + std::to_string(request_.result.code), node.url);
            if (request_.result.code != 200 || request_.result.response->empty()) break;

            auto response = *request_.result.response;
            auto response2 = response;

            if (e_ == "1") {
                auto blocks_array = str::split(response, "\n");
                blocks_array.pop_back(); //empty block
                response2.clear();

                for (auto &block : blocks_array) {
                    auto block_decrypted = rsa.easy_private_decrypt(block);

                    if (block_decrypted.empty()) {
                        if_debug_print(logger::type::error, rsa.get_last_error(), request.path);
                        return;
                    }

                    response2.append(block_decrypted);
                }
            }

            nlohmann::json json = nlohmann::json::parse(response2);
            const auto size = json["count"];

            for (int i = 0; i < size; ++i) {
                const auto result = json["results"][i];
                const auto title = result["title"].get<std::string>();
                const auto url = result["url"].get<std::string>();
                const auto desc = result["desc"].get<std::string>();
                const auto rating = result["rating"].get<size_t>();
                const auto has_ads = result["has_ads"].get<bool>() ? "bad" : "good";
                const auto has_analytics = result["has_analytics"].get<bool>() ? "bad" : "good";
                const auto id = result["id"].get<std::string>();

                std::string result_src = str::format(center_result_src_format, title, url, desc, rating, id, node.url, has_ads, has_analytics);
                center_results_src.append(result_src);
            }

            break;
        }

        std::string center_results_src2 = center_results_src;

        if (e_ == "1") {
            encryption::rsa rsa_client;
            rsa_client.read_public_key_buffer(ek_.data(), ek_.size());
            center_results_src2.clear();

            //https://crypto.stackexchange.com/questions/32692/what-is-the-typical-block-size-in-rsa/32694#32694
            int max_bytes = 100;//256 - 11(too large) //128 - 1024, 256 - 2048
            float size = (float)center_results_src.size() / (float)max_bytes;
            int size_int = (int)size;
            if (size > size_int) ++size_int;

            std::string encrypted_center_results;

            for (int i = 0; i < size_int; ++i) {
                auto result_block = center_results_src.substr(i  * max_bytes, max_bytes);
                auto block_encrypted = rsa_client.easy_public_encrypt(result_block);

                if (block_encrypted.empty()) {
                    if_debug_print(logger::type::error, rsa_client.get_last_error(), request.path);
                    return;
                }

                center_results_src2.append(block_encrypted + "\n");
            }
        }

        std::string url = request.path + params_s;
        str::replace_ref(page_src, "{CENTER_RESULTS}", center_results_src2);
        str::replace_ref(page_src, "{QUERY}", query);
        str::replace_ref(page_src, "{PREV_PAGE}", str::replace(url, "&p=" + p_, "&p=" + std::to_string((page > 1) ? page - 1 : 1)));
        str::replace_ref(page_src, "{NEXT_PAGE}", str::replace(url, "&p=" + p_, "&p=" + std::to_string(page + 1)));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void pages::node_info(const Request &request, Response &response) {
        std::string page_src = config::helper::get_file_content("../frontend/src/node/info.html");

        str::replace_ref(page_src, "{websites_S_COUNT}", std::to_string(get_field_count("host")));
        str::replace_ref(page_src, "{PAGES_COUNT}", std::to_string(get_field_count("url")));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void pages::node_admin_panel(const Request &request, Response &response) {
        std::string page_src = config::helper::get_file_content("../frontend/src/node/admin_panel/index.html");

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void pages::api_get_rsa_public_key(const Request &request, Response &response) {
        response.status = 200;
        response.set_content(rsa.get_public_key_buffer(), "text/html");
    }
    void pages::api_plus_rating(const Request &request, Response &response) {
        const std::string id = request.get_param_value("id");
        const std::string is_redirect = request.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating");

        if (rating < 200) {
            update(id, "rating", rating + 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            response.status = 301;
            const auto referer = request.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            response.set_redirect(redirect_url);
        } else {
            response.status = 200;
        }
    }
    void pages::api_minus_rating(const Request &request, Response &response) {
        const std::string id = request.get_param_value("id");
        const std::string is_redirect = request.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating");

        if (rating > 0) {
            update(id, "rating", rating - 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            response.status = 301;
            const auto referer = request.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            response.set_redirect(redirect_url);
        } else {
            response.status = 200;
        }
    }
    void pages::api_search(const Request &request, Response &response) {
        std::string query = str::replace(request.get_param_value("q"), " ", "+");
        std::string e_ = request.get_param_value("e");
        std::string ek_ = request.get_param_value("ek");
        const std::string p_ = request.get_param_value("p");
        const size_t page = (!p_.empty()) ? std::stoi(p_) : 1;
        nlohmann::json page_src;
        std::vector<unsigned char> ek_decrypted;

        if (e_ == "1") {
            ek_decrypted = encryption::base64::decode(ek_);
            query = rsa.easy_private_decrypt(query);

            if (query.empty()) {
                if_debug_print(logger::type::error, rsa.get_last_error(), request.path);
                return;
            }

            if_debug_print(logger::type::info, "decrypted query = " + query, request.path);
        }

        const auto search_results = search(query, page);

        if (search_results) {
            auto sr_size = search_results->size();
            if_debug_print(logger::type::info, "search_results = " + std::to_string(sr_size), request.path);

            for (int i = 0; i < sr_size; ++i) {
                const auto &result = search_results->operator[](i);
                const size_t title_max_size = 55;   //TODO: CONFIG
                const size_t desc_max_size  = 350;  //TODO: CONFIG

                //remove html tags from text
                std::regex regex(R"(<\/?(\w+)(\s+\w+=(\w+|"[^"]*"|'[^']*'))*(( |)\/|)>)"); //<[^<>]+>
                std::string title = regex_replace(result.title, regex, "");
                std::string desc = regex_replace(result.desc, regex, "");

                if (title.length() > title_max_size) {
                    title = title.substr(0, title_max_size - 3) + "...";
                }
                if (desc.length() > desc_max_size) {
                    desc = desc.substr(0, desc_max_size - 3) + "...";
                }

                page_src["results"][i]["title"] = title;
                page_src["results"][i]["url"] = result.url;
                page_src["results"][i]["desc"] = desc;
                page_src["results"][i]["rating"] = result.rating;
                page_src["results"][i]["id"] = result.id;
                page_src["results"][i]["has_ads"] = result.has_ads;
                page_src["results"][i]["has_analytics"] = result.has_analytics;
            }

            page_src["count"] = search_results->size();
        } else {
            if_debug_print(logger::type::info, "search_results = null", request.path);
        }

        std::string result = page_src.dump();
        std::string result2 = result;

        if (e_ == "1") {
            encryption::rsa rsa_;
            rsa_.read_public_key_buffer(ek_decrypted.data(), ek_decrypted.size());
            result2.clear();

            int max_bytes = 128 - 11; //128 - 1024, 256 - 2048
            float size = (float)result.size() / (float)max_bytes;
            int size_int = (int)size;
            if (size > size_int) ++size_int;

            for (int i = 0; i < size_int; ++i) {
                auto result_block = result.substr(i  * max_bytes, max_bytes);
                auto block_encrypted = rsa_.easy_public_encrypt(result_block);

                if (block_encrypted.empty()) {
                    if_debug_print(logger::type::error, rsa_.get_last_error(), request.path);
                    return;
                }

                result2.append(block_encrypted + "\n");
            }
        }

        response.status = 200;
        response.set_content(result2, "text/html");
    }
    void pages::api_node_info(const Request &request, Response &response) {
        nlohmann::json page_src;

        //page_src["websites_s_count"] = get_field_count("host");
        page_src["pages_count"] = get_field_count("url");

        response.status = 200;
        response.set_content(page_src.dump(), "text/html");
    }
    void pages::not_found(const Request &request, Response &response) {
        response.status = 301;
        response.set_redirect("/home");
    }
}