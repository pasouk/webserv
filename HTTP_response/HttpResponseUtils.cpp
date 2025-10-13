#include "HttpResponse.hpp"

std::string getHeaderValue(const std::string &key, const std::map<std::string, std::string> &headers)
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return "";
}

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

std::string urlDecode(const std::string& str) 
{
    std::string ret;
    char hex[3] = {0};
    for (size_t i = 0; i < str.size(); ++i) 
    {
        if (str[i] == '+') 
        {
            ret += ' ';
        } 
        else if (str[i] == '%' && i + 2 < str.size())
        {
            hex[0] = str[i+1];
            hex[1] = str[i+2];
            ret += static_cast<char>(strtol(hex, NULL, 16));
            i += 2;
        } 
        else 
        {
            ret += str[i];
        }
    }
    return ret;
}