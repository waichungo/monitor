#include "ZipExt.h"
namespace fs = std::filesystem;
namespace zipext
{
    vector<ZipEntry> getZipEntries(struct zip_t *zip)
    {
        vector<ZipEntry> entries;

        if (zip)
        {
            int i, n = zip_entries_total(zip);
            entries.reserve(n);
            for (i = 0; i < n; ++i)
            {
                ZipEntry entry;
                zip_entry_openbyindex(zip, i);
                {
                    entry.Name = zip_entry_name(zip);
                    entry.isDir = zip_entry_isdir(zip);
                    entry.uncompressedSize = zip_entry_size(zip);
                    entry.compressedSize = zip_entry_comp_size(zip);
                }
                entries.push_back(entry);
                zip_entry_close(zip);
            }
            zip_close(zip);
        }
        return entries;
    }

    vector<uint8_t> extractEntryToMemory(struct zip_t *zip, string entryName)
    {
        vector<uint8_t> result;
        if (zip)
        {
            char *buf = NULL;
            size_t bufsize = 0;
            {
                if (zip_entry_open(zip, entryName.c_str()) == 0)
                {
                    if (zip_entry_read(zip, (void **)&buf, &bufsize) > 0)
                    {
                        result.resize(bufsize);
                        memcpy(&result[0], buf, bufsize);
                    }
                }
                zip_entry_close(zip);
            }
            
            zip_stream_close(zip);

            free(buf);
        }
        return result;
    }
    bool extractEntry(struct zip_t *zip, string outFile, string entryName)
    {
        bool result = false;
        if (zip)
        {
            if (zip_entry_open(zip, entryName.c_str()) == 0)
            {
                if (!fs::exists(fs::path(outFile).parent_path()))
                {
                    fs::create_directories(fs::path(outFile).parent_path());
                }
                result = zip_entry_fread(zip, outFile.c_str());
                zip_entry_close(zip);
            }
        }
        return result;
    }
 
    vector<ZipEntry> GetZipEntries(string file)
    {
        vector<ZipEntry> entries;
        struct zip_t *zip = zip_open(file.c_str(), 0, 'r');
        return getZipEntries(zip);
    }

    vector<ZipEntry> GetZipEntries(vector<uint8_t> &buffer)
    {
        vector<ZipEntry> entries;
        struct zip_t *zip = zip_stream_open((const char *)&buffer[0], buffer.size(), 0, 'r');
        return getZipEntries(zip);
    }
    vector<uint8_t> ExtractEntryToMemory(std::string file, string entryName)
    {

        struct zip_t *zip = zip_open(file.c_str(), 0, 'r');
        return extractEntryToMemory(zip, entryName);
    }
    vector<uint8_t> ExtractEntryToMemory(vector<uint8_t> &zipbuffer, string entryName)
    {

        struct zip_t *zip = zip_stream_open((const char *)&zipbuffer[0], zipbuffer.size(), 0, 'r');
        return extractEntryToMemory(zip, entryName);
    }
    bool ExtractEntryToFile(std::string file, string outFile, string entryName)
    {

        struct zip_t *zip = zip_open(file.c_str(), 0, 'r');
        return extractEntry(zip, outFile, entryName);
    }
    bool ExtractEntryToFile(vector<uint8_t> &zipbuffer, string outFile, string entryName)
    {

        struct zip_t *zip = zip_stream_open((const char *)&zipbuffer[0], zipbuffer.size(), 0, 'r');
        return extractEntry(zip, outFile, entryName);
    }
    bool ExtractToFolder(std::string file, string destFolder)
    {
        if (!fs::exists(destFolder))
        {
            fs::create_directories(destFolder);
        }
        return zip_extract(file.c_str(), destFolder.c_str(), NULL, NULL) == 0;
    }
    bool ExtractToFolder(vector<uint8_t> &zipbuffer, string destFolder)
    {
        if (!fs::exists(destFolder))
        {
            fs::create_directories(destFolder);
        }
        struct zip_t *zip = zip_stream_open((const char *)&zipbuffer[0], zipbuffer.size(), 0, 'r');
        auto entries = getZipEntries(zip);
        bool success = true;
        for (auto &entry : entries)
        {
            if (!entry.isDir)
            {
                auto target = fs::path(destFolder);
                target /= entry.Name;
                if (!fs::exists(target.parent_path()))
                {
                    fs::create_directories(target.parent_path());
                }

                success = success && extractEntry(zip, target.string(), entry.Name);
            }
        }

        return success;
    }

}