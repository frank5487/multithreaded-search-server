/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to the students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "./HttpUtils.h"
#include "./FileReader.h"

using std::string;

namespace searchserver {

    bool FileReader::read_file(string *str) {
        // Read the file into memory, and store the file contents in the
        // output parameter "str."  Be careful to handle binary data
        // correctly; i.e., you probably want to use the two-argument
        // constructor to std::string (the one that includes a length as a
        // second argument).

        // TODO: implement

        /*
        int fd = open(fname_.c_str(), O_RDONLY);
        if (fd == -1) {
            return false;
        }

        string line = string();

        int len;
        char buf;

        while (true) {
            len = read(fd, &buf, 1);
            if (len == -1) {
                if (errno != EINTR) {
                    exit(EXIT_FAILURE);
                }
                continue;
            } else if (len == 0) {
                break;
            }
            line += buf;
        }

        (*str) = line;

        close(fd);

        return true;
         */

        std::ifstream file(fname_);
        if (!file.is_open()){
            return false;
        }

        (*str) = static_cast<std::ostringstream&>(std::ostringstream{} << file.rdbuf()).str();

        file.close();

        return true;
    }

}  // namespace searchserver
