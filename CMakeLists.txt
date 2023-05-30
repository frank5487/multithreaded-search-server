cmake_minimum_required(VERSION 3.24)
project(multithreaded_server)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(multithreaded_server
        CrawlFileTree.cc
        CrawlFileTree.h
        FileReader.cc
        FileReader.h
        HttpConnection.cc
        HttpConnection.h
        httpd.cc
        HttpRequest.h
        HttpResponse.h
        HttpServer.cc
        HttpServer.h
        HttpUtils.cc
        HttpUtils.h
        Result.h
        ServerSocket.cc
        ServerSocket.h
        test_crawlfiletree.cc
        test_filereader.cc
        test_httpconnection.cc
        test_httputils.cc
        test_serversocket.cc
        test_suite.cc
        test_suite.h
        test_threadpool.cc
        test_wordindex.cc
        ThreadPool.cc
        ThreadPool.h
        WordIndex.cc
        WordIndex.h)
