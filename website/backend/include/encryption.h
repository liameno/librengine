#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <vector>
#include <memory>

namespace encryption {
    class rsa {
        //https://shanetully.com/2012/04/simple-public-key-encryption-with-rsa-and-openssl/
        //https://gist.github.com/lillypad/f5de36cd9769cfaa54af80b4b331184e
    private:
        RSA *p_rsa;
    public:
        rsa();
        ~rsa();

        int get_size();
        std::string get_last_error();

        void generate_keys(const int &key_length);

        std::string get_public_key_buffer();
        std::string get_private_key_buffer();

        void read_public_key_buffer(void *text, const int &size);
        void read_private_key_buffer(void *text, const int &size);

        void read_public_key_file(const char *path);
        void read_private_key_file(const char *path);

        int public_encrypt(unsigned char *text, const int &size, unsigned char *encrypted);
        int private_encrypt(unsigned char *text, const int &size, unsigned char *encrypted);
        int public_decrypt(unsigned char *text, const int &size, unsigned char *decrypted);
        int private_decrypt(unsigned char *text, const int &size, unsigned char *decrypted);

        std::string easy_public_encrypt(std::string &text);
        std::string easy_private_encrypt(std::string &text);
        std::string easy_public_decrypt(std::string &text);
        std::string easy_private_decrypt(std::string &text);
    };

    namespace base64 {
        static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        static inline bool is_base64(unsigned char c);

        std::string encode(const unsigned char *bytes_to_encode, unsigned int in_len);
        std::vector<unsigned char> decode(const std::string &encoded_string);

        std::string easy_encode(std::string &text);
        std::string easy_decode(std::string &text);
    }
}

#endif