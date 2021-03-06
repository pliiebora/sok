//
// Created by Centauria V. CHEN on 2022/4/8.
//

#include "pack.h"
#include "util.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>


namespace fs = std::filesystem;

std::vector<ResourceItem> scan(const std::string &path)
{
    std::vector<ResourceItem> res{};
    uint64_t offset = 0;
    for (auto &e: fs::recursive_directory_iterator(path))
    {
        auto fp = e.path();
        if (std::regex_search(fp.string(), std::regex{R"(\.DS_Store$)"}))
        {
            continue;
        }
        if (fs::is_regular_file(fp))
        {
            auto size = fs::file_size(fp);
            auto xpath = std::regex_replace(fp.string(), std::regex{"^" + path}, "");
            ltrim(xpath, R"(/\)");
            std::cout << fp << " -> " << xpath << std::endl;
            fs::path f_xpath{xpath};
            ResourceItem item{f_xpath.generic_string(), size, offset};
            res.push_back(item);
            offset += size;
        }
    }
    return res;
}

void write_file(const std::string &path,
                uint16_t signature,
                const std::string &filename,
                const std::vector<ResourceItem> &items)
{
    std::ofstream pack(filename, std::ios::out | std::ios::binary);
    if (pack.is_open())
    {
        ResourceHeader header{signature, static_cast<uint32_t>(items.size())};
        pack.write((char *) (&header), sizeof(ResourceHeader));
        auto pack_buf = std::ostreambuf_iterator<char>(pack);
        for (auto item: items)
        {
            pack.write(item.xpath.c_str(), item.xpath.size() + 1);
            pack.write((char *) (&item.filesize), sizeof(item.filesize));
            pack.write((char *) (&item.start_offset), sizeof(item.start_offset));
        }
        for (const auto &item: items)
        {
            auto item_path = fs::path(path) / item.xpath;
            std::cout << "item path: " << item_path << std::endl;
            std::ifstream reader(item_path, std::ios::in | std::ios::binary);
            std::istreambuf_iterator<char> begin(reader), end;
            std::copy(begin, end, pack_buf);
            reader.close();
        }
        pack.close();
    } else
    {
        std::cerr << "unable to open file " << filename << std::endl;
    }
}
