// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE_SMTPXX
//
// smtpxx - A portable C++ SMTP library for use with netxx
// Copyright 2004 Trustees of Dartmouth College
// 
// All Rights Reserved
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
// 3. Neither the name of the Author nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
// @END_LICENSE_SMTPXX

#ifndef smtpxx_H
#define smtpxx_H

/// smtpxx is a simple SMTP client based on the netxx portable networking
/// library.  It can used to send MIME-encoded e-mails with base64
/// attachments.  smtpxx is intended for use as an error-notification
/// system.
namespace smtpxx {

    class email {
        std::string m_subject;
        std::string m_from;
        std::vector<std::string> m_to;
        std::vector<std::string> m_cc;

    public:
        email(const std::string &subject, const std::string &from)
            : m_subject(subject), m_from(from) {}

        std::string subject() const { return m_subject; }
        std::string from() const { return m_from; }

        void to(const std::string &to) { m_to.push_back(to); }
        std::vector<std::string> to() const { return m_to; }

        void cc(const std::string &cc) { m_cc.push_back(cc); }
        std::vector<std::string> cc() const { return m_cc; }        
    };
};

#endif // smtpxx_H
