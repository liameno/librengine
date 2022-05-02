#include "../include/encryption.h"

namespace encryption {
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
        char *err = static_cast<char *>(malloc(130));
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        return err;
    }

    void rsa::generate_keys(const int &key_length) {
        p_rsa = RSA_generate_key(key_length, 65535, nullptr, nullptr);
    }

    std::string rsa::get_public_key_buffer() {
        BIO *key = BIO_new(BIO_s_mem());
        PEM_write_bio_RSA_PUBKEY(key, p_rsa);

        int length = BIO_pending(key);
        char *s_key = static_cast<char *>(malloc(length + 1));

        BIO_read(key, s_key, length);
        s_key[length] = '\0';

        BIO_free(key);
        return s_key;
    }
    std::string rsa::get_private_key_buffer() {
        BIO *key = BIO_new(BIO_s_mem());
        PEM_write_bio_RSAPrivateKey(key, p_rsa, NULL, NULL, 0, NULL, NULL);

        int length = BIO_pending(key);
        char *s_key = static_cast<char *>(malloc(length + 1));

        BIO_read(key, s_key, length);
        s_key[length] = '\0';

        BIO_free(key);
        return s_key;
    }

    void rsa::read_public_key_buffer(void *text, const int &size) {
        /*BIO *keybio ;
        keybio = BIO_new_mem_buf(text, size);
        p_rsa = PEM_read_bio_RSA_PUBKEY(keybio, &p_rsa, NULL, NULL);*/
        FILE *fp = fmemopen(text, size, "rb");
        PEM_read_RSA_PUBKEY(fp, &p_rsa, nullptr, nullptr);
        fclose(fp);
    }
    void rsa::read_private_key_buffer(void *text, const int &size) {
        /*BIO *keybio ;
        keybio = BIO_new_mem_buf(text, size);
        p_rsa = PEM_read_bio_RSAPrivateKey(keybio, &p_rsa, NULL, NULL);*/
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

    std::string rsa::easy_public_encrypt(std::string &text) {
        auto text_uc = reinterpret_cast<unsigned char *>(text.data());
        auto encrypted = static_cast<unsigned char *>(malloc(get_size()));
        int encrypted_size = public_encrypt(text_uc, text.size(), encrypted);

        if (encrypted_size == -1) {
            return "";
        }

        std::string result = encryption::base64::encode(encrypted, encrypted_size);
        delete[] encrypted;
        return result;
    }
    std::string rsa::easy_private_encrypt(std::string &text) {
        auto text_uc = reinterpret_cast<unsigned char *>(text.data());
        auto encrypted = static_cast<unsigned char *>(malloc(get_size()));
        int encrypted_size = private_encrypt(text_uc, text.size(), encrypted);

        if (encrypted_size == -1) {
            return "";
        }

        std::string result = encryption::base64::encode(encrypted, encrypted_size);
        delete[] encrypted;
        return result;
    }
    std::string rsa::easy_public_decrypt(std::string &text) {
        auto text_uc = base64::decode(text);
        auto decrypted = static_cast<unsigned char *>(malloc(get_size()));
        int decrypted_size = public_decrypt(text_uc.data(), text_uc.size(), decrypted);

        if (decrypted_size == -1) {
            return "";
        }

        std::string result = reinterpret_cast<char *>(decrypted);
        delete[] decrypted;
        return result.substr(0, decrypted_size);
    }
    std::string rsa::easy_private_decrypt(std::string &text) {
        auto text_uc = base64::decode(text);
        auto decrypted = static_cast<unsigned char *>(malloc(get_size()));
        int decrypted_size = private_decrypt(text_uc.data(), text_uc.size(), decrypted);

        if (decrypted_size == -1) {
            return "";
        }

        std::string result = reinterpret_cast<char *>(decrypted);
        delete[] decrypted;
        return result.substr(0, decrypted_size);
    }

    static inline bool base64::is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string base64::encode(const unsigned char *bytes_to_encode, unsigned int in_len) {
        std::string ret;
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

                for(i = 0; (i <4) ; i++) {
                    ret += base64_chars[char_array_4[i]];
                }

                i = 0;
            }
        }

        if (!i) {
            return ret;
        }

        for(j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while((i++ < 3)) {
            ret += '=';
        }

        return ret;

    }
    std::vector<unsigned char> base64::decode(const std::string &encoded_string) {
        size_t in_len = encoded_string.size();
        size_t i = 0;
        size_t j;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<unsigned char> ret;

        while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_];
            in_++;

            if (i ==4) {
                for (i = 0; i <4; i++) {
                    char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++) {
                    ret.push_back(char_array_3[i]);
                }
                i = 0;

            }
        }

        if (!i) {
            return ret;
        }

        for (j = i; j <4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j <4; j++) {
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);

        return ret;
    }

    std::string base64::easy_encode(std::string &text) {
        return encode(reinterpret_cast<unsigned char *>(text.data()), text.size());
    }
    std::string base64::easy_decode(std::string &text) {
        return reinterpret_cast<char *>(decode(text).data());
    }
}