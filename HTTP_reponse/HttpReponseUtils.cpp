#include "HttpResponse.hpp"


bool    isFolder(std::string &path)
{
    struct stat temp;

    if (stat(path.c_str(), &temp) == -1)
        return false;
    else if (S_ISDIR(temp.st_mode))
        return true;
    return false; 
}

bool resourceExists(const std::string& path) 
{
    struct stat temp;

    if (stat(path.c_str(), &temp) == 0) 
        return true;
    return false;
}

std::string toString(size_t n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}