/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:33:02 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/14 08:37:43 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

bool is_elf_binary(const char *path)
{
    unsigned char magic[4];
    FILE *f = fopen(path, "rb");

    if (!f) return false;
    if (fread(magic, 1, 4, f) != 4)
    {
        fclose(f);
        return false;
    }
    fclose(f);
    return magic[0] == 0x7F &&
           magic[1] == 'E' &&
           magic[2] == 'L' &&
           magic[3] == 'F';
}

bool is_macho_binary(const char* path)
{
    uint32_t magic = 0;
    FILE* f = fopen(path, "rb");
    if (!f) return false;

    if (fread(&magic, 1, sizeof(magic), f) != sizeof(magic)) {
        fclose(f);
        return false;
    }
    fclose(f);
    switch (magic) {
        // Mach-O thin
        case 0xFEEDFACE: // MH_MAGIC   (32-bit)
        case 0xFEEDFACF: // MH_MAGIC_64 (64-bit)
        case 0xCEFAEDFE: // MH_CIGAM   (32-bit swapped)
        case 0xCFFAEDFE: // MH_CIGAM_64 (64-bit swapped)

        // FAT binaries
        case 0xCAFEBABE: // FAT_MAGIC
        case 0xBEBAFECA: // FAT_CIGAM
        case 0xCAFEBABF: // FAT_MAGIC_64
        case 0xBFBAFECA: // FAT_CIGAM_64
            return true;
        default:
            return false;
    }
}


bool is_executable(const char *path)
{
    return access(path, X_OK) == 0;
}

bool is_path(const char *path)
{
    struct stat s;
    return stat(path, &s) == 0; 
}

std::string getFilename(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string currentDateTime()
{
    std::time_t now = std::time(NULL);
    std::tm *ltm = std::localtime(&now); // convertit en heure locale
    std::ostringstream oss;
    oss << '['
        << (1900 + ltm->tm_year) << '-'
        << std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << '-'
        << std::setw(2) << std::setfill('0') << ltm->tm_mday << ' '
        << std::setw(2) << std::setfill('0') << ltm->tm_hour << ':'
        << std::setw(2) << std::setfill('0') << ltm->tm_min << ':'
        << std::setw(2) << std::setfill('0') << ltm->tm_sec
        << ']';
    return oss.str();
}

void logOutMessage(std::ostringstream& oss)
{
    std::cout << currentDateTime() << " " << getpid() << " " << getppid() << " " << oss.str() << std::endl;
    oss.str("");
    oss.clear();
}

void logErrMessage(std::ostringstream& oss)
{
    std::cerr << currentDateTime() << " " << getpid() << " " << getppid() << " " << oss.str() << std::endl;
    oss.str("");
    oss.clear();
}

