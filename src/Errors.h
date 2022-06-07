//
// Created by guibertf on 5/30/22.
//

#ifndef TREECORESETPROJ_ERRORS_H
#define TREECORESETPROJ_ERRORS_H

#include <exception>
#include <string>

class UninitializedCoresetException: virtual public std::exception{
protected:
    int error_number;
    int error_offset;
    std::string error_message;

public:
    explicit UninitializedCoresetException(const std::string& msg, int err_num, int err_off):
    error_number(err_num),
    error_offset(err_off),
    error_message(msg){

    }

    virtual ~UninitializedCoresetException() throw() {}

    virtual const char* what() const throw() {
        return error_message.c_str();
    }
    virtual int getErrorNumber() const throw(){
        return error_number;
    }

    virtual int getErrorOffset() const throw(){
        return error_offset;
    }

};

#endif //TREECORESETPROJ_ERRORS_H
