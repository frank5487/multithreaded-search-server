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

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"
#include "./WordIndex.h"


using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

namespace searchserver {
///////////////////////////////////////////////////////////////////////////////
// Constants, internal helper functions
///////////////////////////////////////////////////////////////////////////////
    static const char *kFivegleStr =
            "<html><head><title>5950gle</title></head>\n"
            "<body>\n"
            "<center style=\"font-size:500%;\">\n"
            "<span style=\"position:relative;bottom:-0.33em;color:orange;\">5</span>"
            "<span style=\"color:red;\">9</span>"
            "<span style=\"color:gold;\">5</span>"
            "<span style=\"color:blue;\">g</span>"
            "<span style=\"color:green;\">l</span>"
            "<span style=\"color:red;\">e</span>\n"
            "</center>\n"
            "<p>\n"
            "<div style=\"height:20px;\"></div>\n"
            "<center>\n"
            "<form action=\"/query\" method=\"get\">\n"
            "<input type=\"text\" size=30 name=\"terms\" />\n"
            "<input type=\"submit\" value=\"Search\" />\n"
            "</form>\n"
            "</center><p>\n";

// static
    const int HttpServer::kNumThreads = 100;

// This is the function that threads are dispatched into
// in order to process new client connections.
    static void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
    static HttpResponse ProcessRequest(const HttpRequest &req,
                                       const string &base_dir,
                                       WordIndex *indices);

// Process a file request.
    static HttpResponse ProcessFileRequest(const string &uri,
                                           const string &base_dir);

// Process a query request.
    static HttpResponse ProcessQueryRequest(const string &uri,
                                            WordIndex *index);


///////////////////////////////////////////////////////////////////////////////
// HttpServer
///////////////////////////////////////////////////////////////////////////////
    bool HttpServer::run(void) {
        // Create the server listening socket.
        int listen_fd;
        cout << "  creating and binding the listening socket..." << endl;
        if (!socket_.bind_and_listen(&listen_fd)) {
            cerr << endl << "Couldn't bind to the listening socket." << endl;
            return false;
        }

        // Spin, accepting connections and dispatching them.  Use a
        // threadpool to dispatch connections into their own thread.
        cout << "  accepting connections..." << endl << endl;
        ThreadPool tp(kNumThreads);
        while (1) {
            HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
            hst->base_dir = static_file_dir_path_;
            hst->index = index_;
            if (!socket_.accept_client(&hst->client_fd,
                                       &hst->c_addr,
                                       &hst->c_port,
                                       &hst->c_dns,
                                       &hst->s_addr,
                                       &hst->s_dns)) {
                // The accept failed for some reason, so quit out of the server.
                // (Will happen when kill command is used to shut down the server.)
                break;
            }
            // The accept succeeded; dispatch it.
            tp.dispatch(hst);
        }
        return true;
    }

    static void HttpServer_ThrFn(ThreadPool::Task *t) {
        // Cast back our HttpServerTask structure with all of our new
        // client's information in it.
        unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
        cout << "  client " << hst->c_dns << ":" << hst->c_port << " "
             << "(IP address " << hst->c_addr << ")" << " connected." << endl;

        // Read in the next request, process it, write the response.

        // Use the HttpConnection class to read and process the next
        // request from our current client, then write out our response.  If
        // the client sends a "Connection: close\r\n" header, then shut down
        // the connection -- we're done.
        //
        // Hint: the client can make multiple requests on our single connection,
        // so we should keep the connection open between requests rather than
        // creating/destroying the same connection repeatedly.

        // TODO: Implement
        bool done = false;
        HttpConnection conn(hst->client_fd);
        while (!done) {
            HttpRequest req;
            if (!conn.next_request(&req)) {
                close(hst->client_fd);
                break;
            }

            HttpResponse resp = ProcessRequest(req, hst->base_dir, hst->index);
            if (!conn.write_response(resp)) {
                close(hst->client_fd);
                break;
            }
            if (req.GetHeaderValue("Connection").compare("close") == 0) {
                close(hst->client_fd);
                done = true;
            }
        }
    }

    static HttpResponse ProcessRequest(const HttpRequest &req,
                                       const string &base_dir,
                                       WordIndex *index) {
        // Is the user asking for a static file?
        if (req.uri().substr(0, 8) == "/static/") {
            return ProcessFileRequest(req.uri(), base_dir);
        }

        // The user must be asking for a query.
        return ProcessQueryRequest(req.uri(), index);
    }

    static HttpResponse ProcessFileRequest(const string &uri,
                                           const string &base_dir) {
        // The response we'll build up.
        HttpResponse ret;

        // Steps to follow:
        //  - use the URLParser class to figure out what filename
        //    the user is asking for. Note that we identify a request
        //    as a file request if the URI starts with '/static/'
        //
        //  - use the FileReader class to read the file into memory
        //
        //  - copy the file content into the ret.body
        //
        //  - depending on the file name suffix, set the response
        //    Content-type header as appropriate, e.g.,:
        //      --> for ".html" or ".htm", set to "text/html"
        //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
        //      --> for ".png", set to "image/png"
        //      etc.
        //    You should support the file types mentioned above,
        //    as well as ".txt", ".js", ".css", ".xml", ".gif",
        //    and any other extensions to get bikeapalooza
        //    to match the solution server.
        //
        // be sure to set the response code, protocol, and message
        // in the HttpResponse as well.
        string file_name = "";

        // TODO: Implement
        URLParser urlParser;
        urlParser.parse(uri);
        file_name += urlParser.path();
        if (file_name.substr(0, 8).compare("/static/") == 0) {
            file_name = file_name.substr(8);
        }

        FileReader fileReader(file_name);
        string body;
        if (!fileReader.read_file(&body)) {
            // If you couldn't find the file, return an HTTP 404 error.

            ret.set_protocol("HTTP/1.1");
            ret.set_response_code(404);
            ret.set_message("Not Found");
            ret.AppendToBody("<html><body>Couldn't find file \""
                             + escape_html(file_name)
                             + "\"</body></html>\n");
            return ret;
        }

        vector<string> lst;
        boost::split(lst, body, boost::is_any_of("."));

        string suffix = lst[lst.size() - 1];

        if (suffix.compare("html") || suffix.compare("htm")) {
            ret.set_content_type("text/html");
        } else if (suffix.compare("jpeg") == 0 || suffix.compare("jpg") == 0) {
            ret.set_content_type("image/jpeg");
        } else if (suffix.compare("png") == 0) {
            ret.set_content_type("image/png");
        } else if (suffix.compare("txt") == 0) {
            ret.set_content_type("text/plain");
        } else if (suffix.compare("js") == 0) {
            ret.set_content_type("text/javascript");
        } else if (suffix.compare("css") == 0) {
            ret.set_content_type("text/css");
        } else if (suffix.compare("xml") == 0) {
            ret.set_content_type("application/xml");
        } else if (suffix.compare("gif") == 0) {
            ret.set_content_type("image/gif");
        } else if (suffix.compare("csv") == 0) {
            ret.set_content_type("text/csv");
        } else if (suffix.compare("doc") == 0) {
            ret.set_content_type("application/msword");
        } else if (suffix.compare("gz") == 0) {
            ret.set_content_type("application/gzip");
        } else if (suffix.compare("ics") == 0) {
            ret.set_content_type("text/calendar");
        } else if (suffix.compare("pdf") == 0) {
            ret.set_content_type("application/pdf");
        } else if (suffix.compare("tif") == 0 || suffix.compare("tiff") == 0) {
            ret.set_content_type("image/tiff");
        } else if (suffix.compare("rtf") == 0) {
            ret.set_content_type("application/rtf");
        } else if (suffix.compare("sh") == 0) {
            ret.set_content_type("application/x-sh");
        } else {
            ret.set_content_type("text/plain");
        }

        ret.set_protocol("HTTP/1.1");
        ret.set_response_code(200);
        ret.set_message("OK");
        ret.AppendToBody(body);

        return ret;
    }

    static HttpResponse ProcessQueryRequest(const string &uri,
                                            WordIndex *index) {
        // The response we're building up.
        HttpResponse ret;

        // Your job here is to figure out how to present the user with
        // the same query interface as our solution_binaries/httpd server.
        // A couple of notes:
        //
        //  - no matter what, you need to present the 5950gle logo and the
        //    search box/button
        //
        //  - if the user sent in a search query, you also
        //    need to display the search results. You can run the solution binaries to see how these should look
        //
        //  - you'll want to use the URLParser to parse the uri and extract
        //    search terms from a typed-in search query.  convert them
        //    to lower case.
        //
        //  - Use the specified index to generate the query results

        // TODO: implement
        // logo + search box
        ret.AppendToBody(string(kFivegleStr));

        URLParser parser;
        parser.parse(uri);
        string termArgs = parser.args()["terms"];
        boost::to_lower(termArgs);
        boost::trim(termArgs);

        if (!termArgs.empty()) {
            vector<string> terms;
            boost::split(terms, termArgs, boost::is_any_of(" "));
            vector<Result> result = index->lookup_query(terms);
            int resCount = result.size();
            if (resCount == 0) {
                ret.AppendToBody("<p>");
                ret.AppendToBody("No results...");
                ret.AppendToBody("<b>");
                ret.AppendToBody(escape_html(termArgs));
                ret.AppendToBody("</b>");
                ret.AppendToBody("</p>");
            } else {
                //xx results found for
                ret.AppendToBody("<p>");
                ret.AppendToBody(std::to_string(resCount));
                ret.AppendToBody(" results: ");
                ret.AppendToBody("<b>");
                ret.AppendToBody(escape_html(termArgs));
                ret.AppendToBody("</b>\n");
                ret.AppendToBody("</p>\n");
                ret.AppendToBody("<ol>\n");
                for (auto res: result) {
                    ret.AppendToBody("<li>\n");
                    ret.AppendToBody("<a href=\"");
                    if (res.doc_name.substr(0, 7).compare("http://") != 0) {
                        ret.AppendToBody("/static/");
                    }
                    ret.AppendToBody(res.doc_name);
                    ret.AppendToBody("\">");
                    ret.AppendToBody(escape_html(res.doc_name));
                    ret.AppendToBody("</a>");
                    ret.AppendToBody(" [");
                    ret.AppendToBody(std::to_string(res.rank));
                    ret.AppendToBody("]\n");
                    ret.AppendToBody("</li>\n");
                }
                ret.AppendToBody("</ol>\n");
            }
        }

        ret.AppendToBody("</body>\n");
        ret.AppendToBody("</html>\n");
        ret.set_protocol("HTTP/1.1");
        ret.set_response_code(200);
        ret.set_message("OK");
        return ret;
    }

}  // namespace searchserver
