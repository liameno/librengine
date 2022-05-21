#include "../include/encryption.h"

#include "../include/str.h"
#include "../include/logger.h"

namespace librengine::encryption {
    rsa::rsa() {
        p_rsa = RSA_new();
    }

    rsa::~rsa() {
        RSA_free(p_rsa);
    }

    int rsa::get_size() {
        return RSA_size(p_rsa);
    }

    std::string rsa::get_last_error() {
        auto error = new char[130];
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), error);

        std::string result = error;
        delete[] error;

        return result;
    }
    void rsa::generate_keys(const int &key_length) {
        p_rsa = RSA_generate_key(key_length, 65535, nullptr, nullptr);

        //      ERROR (RSA_generate_key_ex)       //
        /*BIO	*bp_public = nullptr;
        BIO *bp_private = nullptr;

        auto bn = BN_new();
        BN_set_word(bn,RSA_F4);

        RSA_generate_key_ex(p_rsa, key_length, bn, nullptr);

        PEM_write_bio_RSAPublicKey(bp_public, p_rsa);
        PEM_write_bio_RSAPrivateKey(bp_private, p_rsa, nullptr, nullptr, 0, nullptr, nullptr);

        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        BN_free(bn);*/
        //      ===========================       //
    }

    std::string rsa::get_public_key_buffer() {
        BIO *key = BIO_new(BIO_s_mem());
        PEM_write_bio_RSA_PUBKEY(key, p_rsa);

        int length = BIO_pending(key);
        char *s_key = static_cast<char *>(malloc(length + 1));

        BIO_read(key, s_key, length);
        s_key[length] = '\0';

        BIO_free(key);
        std::string str = s_key;
        free(s_key);
        return str;
    }

    std::string rsa::get_private_key_buffer() {
        BIO *key = BIO_new(BIO_s_mem());
        PEM_write_bio_RSAPrivateKey(key, p_rsa, NULL, NULL, 0, NULL, NULL);

        int length = BIO_pending(key);
        char *s_key = static_cast<char *>(malloc(length + 1));

        BIO_read(key, s_key, length);
        s_key[length] = '\0';

        BIO_free(key);
        std::string str = s_key;
        free(s_key);
        return str;
    }

    void rsa::read_public_key_buffer(void *text, const int &size) {
        FILE *fp = fmemopen(text, size, "rb");
        PEM_read_RSA_PUBKEY(fp, &p_rsa, nullptr, nullptr);
        fclose(fp);
    }

    void rsa::read_private_key_buffer(void *text, const int &size) {
        FILE *fp = fmemopen(text, size, "rb");
        PEM_read_RSAPrivateKey(fp, &p_rsa, nullptr, nullptr);
        fclose(fp);
    }

    void rsa::read_public_key_file(const char *path) {
        FILE *fp = fopen(path, "rb");
        PEM_read_RSAPublicKey(fp, &p_rsa, nullptr, nullptr);
        fclose(fp);
    }

    void rsa::read_private_key_file(const char *path) {
        FILE *fp = fopen(path, "rb");
        PEM_read_RSAPrivateKey(fp, &p_rsa, nullptr, nullptr);
        fclose(fp);
    }

    int rsa::public_encrypt(unsigned char *text, const int &size, unsigned char *encrypted) {
        return RSA_public_encrypt(size, text, encrypted, p_rsa, RSA_PKCS1_PADDING);
    }

    int rsa::private_encrypt(unsigned char *text, const int &size, unsigned char *encrypted) {
        return RSA_private_encrypt(size, text, encrypted, p_rsa, RSA_PKCS1_PADDING);
    }

    int rsa::public_decrypt(unsigned char *text, const int &size, unsigned char *decrypted) {
        return RSA_public_decrypt(size, text, decrypted, p_rsa, RSA_PKCS1_PADDING);
    }

    int rsa::private_decrypt(unsigned char *text, const int &size, unsigned char *decrypted) {
        return RSA_private_decrypt(size, text, decrypted, p_rsa, RSA_PKCS1_PADDING);
    }

    std::vector<std::string> rsa::to_blocks(const std::string &text, const int &block_size) {
        auto size = (float) text.size() / (float) block_size;
        auto size_int = (int) size;
        if (size > size_int) ++size_int;

        std::vector<std::string> result;
        result.reserve(size_int);

        for (int i = 0; i < size_int; ++i) {
            result.push_back(text.substr(i * block_size, block_size));
        }

        return result;
    }


    void rsa::easy_read_public_key_buffer(const std::string &text) {
        read_public_key_buffer((unsigned char*)text.c_str(), text.size());
    }

    void rsa::easy_read_private_key_buffer(const std::string &text) {
        read_private_key_buffer((unsigned char*)text.c_str(), text.size());
    }

    std::string rsa::easy_public_encrypt(const std::string &text) {
        auto block_size = get_size();
        std::string result;

        auto blocks = to_blocks(text, block_size - 11);
        auto size = blocks.size();

        for (int i = 0; i < size; ++i) {
            auto block = blocks[i];
            auto text_uc = (unsigned char*)block.c_str();
            auto encrypted = new unsigned char[block_size];
            int encrypted_size = public_encrypt(text_uc, block.size(), encrypted);

            if (encrypted_size == -1) {
                delete[] encrypted;
                return "";
            }

            auto base64 = encryption::base64::encode(encrypted, encrypted_size);
            result.append(base64);
            if (i != size - 1) result += "\n";
            delete[] encrypted;
        }

        return result;
    }

    std::string rsa::easy_private_encrypt(const std::string &text) {
        auto block_size = get_size();
        std::string result;

        auto blocks = to_blocks(text, block_size - 11);
        auto size = blocks.size();

        for (int i = 0; i < size; ++i) {
            auto block = blocks[i];
            auto text_uc = (unsigned char*)block.c_str();
            auto encrypted = new unsigned char[block_size];
            int encrypted_size = private_encrypt(text_uc, block.size(), encrypted);

            if (encrypted_size == -1) {
                delete[] encrypted;
                return "";
            }

            auto base64 = encryption::base64::encode(encrypted, encrypted_size);
            result.append(base64);
            if (i != size - 1) result += "\n";
            delete[] encrypted;
        }

        return result;
    }

    std::string rsa::easy_public_decrypt(const std::string &text) {
        auto block_size = get_size();
        std::string result;

        auto blocks = str::split(text, "\n");
        auto size = blocks.size();

        for (int i = 0; i < size; ++i) {
            auto block = blocks[i];
            auto text_uc = base64::decode(block);
            auto decrypted = new unsigned char[block_size];
            int decrypted_size = public_decrypt(text_uc.data(), text_uc.size(), decrypted);

            if (decrypted_size == -1) {
                delete[] decrypted;
                return "";
            }

            auto s = ((std::string)(char*)(decrypted)).substr(0, decrypted_size);
            result.append(s);
            delete[] decrypted;
        }

        return result;
    }

    std::string rsa::easy_private_decrypt(const std::string &text) {
        auto block_size = get_size();
        std::string result;

        auto blocks = str::split(text, "\n");
        auto size = blocks.size();

        for (int i = 0; i < size; ++i) {
            auto block = blocks[i];
            auto text_uc = base64::decode(block);
            auto decrypted = new unsigned char[block_size];
            int decrypted_size = private_decrypt(text_uc.data(), text_uc.size(), decrypted);

            if (decrypted_size == -1) {
                delete[] decrypted;
                return "";
            }

            auto s = ((std::string)(char*)(decrypted)).substr(0, decrypted_size);
            result.append(s);
            delete[] decrypted;
        }

        return result;
    }

    std::string base64::encode(const unsigned char *bytes_to_encode, unsigned int in_len) {
        std::string result;
        int i = 0;
        int j;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);

            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i < 4) ; i++) {
                    result += base64_chars[char_array_4[i]];
                }

                i = 0;
            }
        }

        if (!i) return result;
        for(j = i; j < 3; j++) char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) result += base64_chars[char_array_4[j]];
        while((i++ < 3)) result += '=';

        return result;
    }

    std::vector<unsigned char> base64::decode(const std::string &encoded_string) {
        size_t in_len = encoded_string.size();
        size_t i = 0;
        size_t j;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<unsigned char> result;

        while (in_len-- && (encoded_string[in_] != '=') && (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
            char_array_4[i++] = encoded_string[in_];
            in_++;

            if (i == 4) {
                for (i = 0; i <4; i++) {
                    char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++) {
                    result.push_back(char_array_3[i]);
                }

                i = 0;
            }
        }

        if (!i) return result;
        for (j = i; j <4; j++) char_array_4[j] = 0;
        for (j = 0; j <4; j++) char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) result.push_back(char_array_3[j]);
        return result;
    }

    std::string base64::easy_encode(std::string &text) {
        return encode((unsigned char*)text.c_str(), text.size());
    }

    std::string base64::easy_decode(std::string &text) {
        auto decoded = decode(text);
        return { decoded.begin(), decoded.end() };
    }
}