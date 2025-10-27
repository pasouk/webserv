#include "ParserHttp.hpp"

int ft_countwords(std::string str)
{
    int words = 0;
    int i = 1;

    if (str.empty())
        return 0;
    while (str[i] && str[i] != '\r')
    {
        if ((str[i - 1] && str[i] != ' ' && str[i - 1] == ' ')\
            || (words == 0 && str[i] != ' '))
        {
            words++;
        }
            i++;
    }
    return words;
}

std::string trim(const std::string& s) 
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) 
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}