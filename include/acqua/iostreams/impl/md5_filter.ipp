#pragma once

extern "C" {
#include <openssl/md5.h>
}

#include <acqua/iostreams/md5_filter.hpp>

namespace acqua { namespace iostreams {

struct md5_context::impl
{
    explicit impl(buffer_type & buffer_)
        : buffer(buffer_) {}

    MD5_CTX context;
    buffer_type & buffer;
};


inline md5_context::md5_context(buffer_type & buffer)
    : impl_(new impl(buffer)) {}


inline void md5_context::init(boost::system::error_code & ec)
{
    impl_->buffer.fill(0);
    int rc = MD5_Init(&impl_->context);
    if (rc != 1) {
        // TODO;
        ec = make_error_code(boost::system::errc::bad_address);
    }
}


inline void md5_context::update(char const * s, std::size_t n, boost::system::error_code & ec)
{
    int rc = MD5_Update(&impl_->context, s, n);
    if (rc != 1) {
        // TODO:
        ec = make_error_code(boost::system::errc::bad_address);
    }
}


inline void md5_context::finish(boost::system::error_code & ec)
{
    int rc = MD5_Final(static_cast<unsigned char *>(impl_->buffer.begin()), &impl_->context);
    if (rc != 1) {
        // TODO:
        ec = make_error_code(boost::system::errc::bad_address);
    }
}

} }
