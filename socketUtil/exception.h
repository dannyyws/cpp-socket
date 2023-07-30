#include <exception>
#include <cstring>

struct CannotCreateSocketException : std::exception
{};

struct NotImplementedException: std::exception
{
    NotImplementedException() = delete;
    NotImplementedException(const char* info)
    {
        feature = new char [strlen(info) + strlen(MESSAGE)];
        strncpy(feature, MESSAGE, strlen(MESSAGE));
        strcat(feature, info);
    }

    ~NotImplementedException()
    {
        delete[] feature;
    }

    virtual const char* what()
    {
        return feature;
    }

private:
    char* feature{};
    static const char* MESSAGE;
};

const char* NotImplementedException::MESSAGE {"Feature is not implemented exception:\n\t\0"};