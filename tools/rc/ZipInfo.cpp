/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

 =============================================================================*/

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "miniz.h"

/*
* @brief Struct which represents an entry in the zip archive.
*/
struct EntryInfo
{
  enum class EntryType {FILE, DIRECTORY};

  std::string name;
  EntryType type;
  mz_uint64 compressedSize;
  mz_uint64 uncompressedSize;
  mz_uint32 crc32;
};

/*
* @brief Read a zip archive and return a vector of EntryInfo objects
* @param filename is the path of the filename
* @returns a vector of EntryInfo objects
* @throw std::runtime exception if zip archive couldn't be read
*/
std::vector<EntryInfo> readZipFile(std::string filename)
{
  std::vector<EntryInfo> entries;
  mz_zip_archive ziparchive;
  memset(&ziparchive, 0, sizeof(mz_zip_archive));
  if (!mz_zip_reader_init_file(&ziparchive, filename.c_str(), 0))
  {
    throw std::runtime_error("Could not read zip archive file " + filename);
  }

  mz_uint numindices = mz_zip_reader_get_num_files(&ziparchive);
  for (mz_uint index = 0; index < numindices; ++index)
  {
    mz_zip_archive_file_stat filestat;
    EntryInfo entry;
    mz_zip_reader_file_stat(&ziparchive, index, &filestat);

    entry.name = filestat.m_filename;
    entry.type = mz_zip_reader_is_file_a_directory(&ziparchive, index) ? \
      EntryInfo::EntryType::DIRECTORY : EntryInfo::EntryType::FILE;
    entry.compressedSize = filestat.m_comp_size;
    entry.uncompressedSize = filestat.m_uncomp_size;
    entry.crc32 = filestat.m_crc32;
    entries.push_back(entry);
  }

  return entries;
}

std::string getStrRepEntries(const std::vector <EntryInfo>& entries)
{
  std::string retStr;

  auto file_or_dir = [](const EntryInfo::EntryType& type) {
    return (type == EntryInfo::EntryType::FILE) ? "File" : "Directory";
  };

  for (const auto &entry : entries)
  {
    retStr += file_or_dir(entry.type) + std::string(",") + std::to_string(entry.compressedSize) + ",";
    retStr += std::to_string(entry.uncompressedSize) + "," + std::to_string(entry.crc32) + ",";
    retStr += entry.name + ",";
  }

  return retStr;
}

/*
* @brief Output EntryInfo objects to stdout in a CSV format
* @param entries is a vector of EntryInfo objects
*/
void outputEntries(const std::vector<EntryInfo>& entries)
{
  std::cout << getStrRepEntries(entries);

}

int main(int argc, char* argv[])
{
  auto print_usage_and_exit = [](char* argv[]){
    std::cout << "Simple listing of the files inside a zip archive" << std::endl;
    std::cout << "Usage: " << argv[0] << " ZIP_PATH " << "COMPARE_STRING " << std::endl;
    std::cout << "ZIP_PATH: The path of the zip file." << std::endl;
    std::cout << "COMPARE_STRING (optional): A string separated by double quotes ";
    std::cout << "which is compared to the stdout generated if the program was invoked ";
    std::cout << "with just the first two arguments. " << std::endl;
    std::cout << "This facilitates testing." << std::endl;
    std::cout << std::endl;
    std::cout << "Example usage:" << std::endl;
    std::cout << "1. " << argv[0] << " Example.zip" << std::endl;
    std::cout << "This example lists the following values of every entry inside the zip ";
    std::cout << "in a CSV-format (One entry per line)." << std::endl;
    std::cout << "FileOrDir, compressed_size, uncompressed_size, crc32, name" << std::endl;
    std::cout << std::endl;
    std::cout << "2. " << argv[0] << " Example.zip \"File,79,102,3826964650,manifest.json\"" << std::endl;
    std::cout << "In this example, the output is PASS if the CSV values agree with the zip contents, ";
    std::cout << "Otherwise the output is FAIL.";
     
    exit(EXIT_FAILURE);
  };

  if (argc != 2 && argc != 3)
  {
    print_usage_and_exit(argv);
  }

  try
  {
    auto entries = readZipFile(argv[1]);
    if (argc == 3)
    {
      std::string testStr(argv[2]);
      std::string refStr = getStrRepEntries(entries);
      (testStr == refStr) ? std::cout << "PASS" : std::cout << "FAIL" << std::endl;
      std::cout << refStr << std::endl;
      std::cout << testStr << std::endl;
    }
    else
    {
      outputEntries(entries);
    }
  }
  catch (std::runtime_error)
  {
    std::cerr << "Error reading the zip archive: " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}