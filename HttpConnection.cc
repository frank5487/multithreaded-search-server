/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <cstdint>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace searchserver {

    static const char *kHeaderEnd = "\r\n\r\n";
    static const int kHeaderEndLen = 4;

    bool HttpConnection::next_request(HttpRequest *request) {
        // Use "wrapped_read" to read data into the buffer_
        // instance variable.  Keep reading data until either the
        // connection drops or you see a "\r\n\r\n" that demarcates
        // the end of the request header.
        //
        // Once you've seen the request header, use parse_request()
        // to parse the header into the *request argument.
        //
        // Very tricky part:  clients can send back-to-back requests
        // on the same socket.  So, you need to preserve everything
        // after the "\r\n\r\n" in buffer_ for the next time the
        // caller invokes next_request()!

        // TODO: implement

        size_t pos = buffer_.find(kHeaderEnd, 0, kHeaderEndLen);
        if (pos == string::npos) {
            while (true) {
                string t = "";
                int r = wrapped_read(fd_, &t);
                if (r <= 0) {
                    return false;
                }
                buffer_ += t;
                pos = buffer_.find(kHeaderEnd, 0, kHeaderEndLen);
                if (pos != string::npos) {
                    break;
                }
            }
        }

        bool res;
        if (pos < buffer_.length() - 4) {
            res = parse_request(buffer_.substr(0, pos + kHeaderEndLen), request);
            buffer_ = buffer_.substr(pos + kHeaderEndLen);
        } else {
            res = parse_request(buffer_, request);
            buffer_ = "";
        }

        return res;
    }

    bool HttpConnection::write_response(const HttpResponse &response) {
        // Implement so that the response is converted to a string
        // and written out to the socket for this connection

        // TODO: implement
        string resp = response.GenerateResponseString();
        if (wrapped_write(fd_, resp) == -1) {
            return false;
        }

        return true;
    }

    bool HttpConnection::parse_request(const string &request, HttpRequest *out) {
        HttpRequest req("/");  // by default, get "/".

        // Split the request into lines.  Extract the URI from the first line
        // and store it in req.URI.  For each additional line beyond the
        // first, extract out the header name and value and store them in
        // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
        // at HttpRequest.h for details about the HTTP header format that
        // you need to parse.
        //
        // You'll probably want to look up boost functions for (a) splitting
        // a string into lines on a "\r\n" delimiter, (b) trimming
        // whitespace from the end of a string, and (c) converting a string
        // to lowercase.
        //
        // If a request is malfrormed, return false, otherwise true and
        // the parsed request is retrned via *out

        // TODO: implement

        // extract each line
        vector<string> lines;
        boost::split(lines, request, boost::is_any_of("\r\n"));
        for (auto &line: lines) {
            boost::trim(line);
        }

        // extract uri from the first line
        string first_line = lines[0];
        vector<string> elements;
        boost::split(elements, first_line, boost::is_any_of(" "));
        // malformed
        if (elements.size() < 3) {
            return false;
        }
        boost::trim(elements[1]);
        boost::to_lower(elements[1]);
        req.set_uri(elements[1]);

        // parse header
        for (size_t i = 1; i < lines.size(); i++) {
            string line = lines[i];
            vector<string> eles;
            boost::split(eles, line, boost::is_any_of(":"));
            if (eles.size() < 2) {
                continue;
            }
            string header = eles[0];
            string content = eles[1];
            boost::trim(header);
            boost::to_lower(header);
            boost::trim(content);
            req.AddHeader(header, content);

//            string line = lines[i];
//            size_t pos = line.find(":");
//            if (pos == string::npos) {
//                continue;
//            }
//            string name = line.substr(0, pos);
//            string value = line.substr(pos + 2, line.length());
//            boost::trim(name);
//            boost::to_lower(name);
//            req.AddHeader(name, value);
        }

        *out = req;
        return true;

    }

}  // namespace searchserver
