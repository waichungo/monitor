#pragma once
#include <zip.h>
#include <string>
#include <vector>
#include <filesystem>
using std::string;
using std::vector;
namespace zipext
{
    class ZipEntry
    {
    public:
        string Name;
        size_t uncompressedSize;
        size_t compressedSize;
        bool isDir;
    };
    class MemoryEntry
    {
    public:
        string entryName;
        vector<uint8_t> data;
    };
    class ZipCreator
    {
    private:
        bool createFromDir = false;
        bool createFromMemEntries = false;

    public:
        std::string inputFolder;
        std::vector<std::string> files;
        std::vector<MemoryEntry> memEntries;

        bool SaveToFile(std::string outFile)
        {
            bool result = false;
            struct zip_t *zip = zip_open(outFile.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
            if (zip)
            {
                bool success = true;

                if (createFromDir)
                {
                    bool status = true;
                    for (auto path : std::filesystem::recursive_directory_iterator(inputFolder, std::filesystem::directory_options::skip_permission_denied))
                    {

                        try
                        {
                            if (path.exists() && path.is_regular_file())
                            {
                                auto entryName = path.path().string().substr(inputFolder.size() + 1);

                                if ((success = zip_entry_open(zip, entryName.c_str())) == 0)
                                {
                                    status = status && zip_entry_fwrite(zip, path.path().string().c_str()) == 0;
                                }
                                else
                                {
                                    status = false;
                                }
                                zip_entry_close(zip);
                            }
                        }
                        catch (const std::exception &e)
                        {
                        }
                    }
                    result = status;
                }
                else if (createFromMemEntries)
                {
                    bool status = true;
                    for (auto &entry : memEntries)
                    {
                        if (zip_entry_open(zip, entry.entryName.c_str()) == 0)
                        {
                            status = zip_entry_write(zip, &entry.data[0], entry.data.size()) == 0;
                        }
                        else
                        {
                            status = false;
                        }
                        zip_entry_close(zip);
                    }
                    result = status;
                }
                else
                {
                    bool status = true;
                    for (auto &file : files)
                    {
                        auto name = std::filesystem::path(file).filename().string();
                        if ((success = zip_entry_open(zip, name.c_str())) == 0)
                        {
                            status = status && zip_entry_fwrite(zip, file.c_str()) == 0;
                        }
                        else
                        {
                            status = false;
                        }

                        zip_entry_close(zip);
                    }
                    result = status;
                }
                zip_close(zip);
            }

            return result;
        }
        vector<uint8_t> SaveToMemory()
        {
            inputFolder=std::filesystem::absolute(inputFolder).string();
            vector<uint8_t> buffer;
            bool result = false;
            char *outbuf = NULL;
            size_t outbufsize = 0;
            struct zip_t *zip = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
            if (zip)
            {
                bool success = true;

                if (createFromDir)
                {
                    bool status = true;
                    for (auto path : std::filesystem::recursive_directory_iterator(inputFolder, std::filesystem::directory_options::skip_permission_denied))
                    {

                        try
                        {
                            if (path.exists() && path.is_regular_file())
                            {
                                auto file = std::filesystem::absolute(path.path()).string();
                                auto entryName = file.substr(inputFolder.size() + 1);

                                if ((success = zip_entry_open(zip, entryName.c_str())) == 0)
                                {
                                    status = status && zip_entry_fwrite(zip, file.c_str()) == 0;
                                }
                                else
                                {
                                    status = false;
                                }
                                zip_entry_close(zip);
                            }
                        }
                        catch (const std::exception &e)
                        {
                        }
                    }
                    result = status;
                }
                else if (createFromMemEntries)
                {
                    bool status = true;
                    for (auto &entry : memEntries)
                    {
                        if (zip_entry_open(zip, entry.entryName.c_str()) == 0)
                        {
                            status = zip_entry_write(zip, &entry.data[0], entry.data.size()) == 0;
                        }
                        else
                        {
                            status = false;
                        }
                        zip_entry_close(zip);
                    }
                    result = status;
                }
                else
                {
                    bool status = true;
                    for (auto &file : files)
                    {
                        auto name = std::filesystem::path(file).filename().string();
                        if ((success = zip_entry_open(zip, name.c_str())) == 0)
                        {
                            status = status && zip_entry_fwrite(zip, file.c_str()) == 0;
                        }
                        else
                        {
                            status = false;
                        }

                        zip_entry_close(zip);
                    }
                    result = status;
                }
                if (result)
                {
                    zip_stream_copy(zip, (void **)&outbuf, &outbufsize);
           
                    buffer.resize(outbufsize);
                    memcpy(&buffer[0], outbuf, outbufsize);
                    free(outbuf);
                    zip_stream_close(zip);
                }else{
                zip_close(zip);
                }
            }

            return buffer;
        }
        static ZipCreator FromFiles(std::vector<std::string> files)
        {
            ZipCreator creator;
            creator.files = files;
            creator.createFromDir = false;
            creator.createFromMemEntries = false;
            return creator;
        }
        static ZipCreator FromMemEntries(std::vector<MemoryEntry> &memEntries)
        {
            ZipCreator creator;
            creator.memEntries = memEntries;
            creator.createFromDir = false;
            creator.createFromMemEntries = true;
            return creator;
        }
        static ZipCreator FromDirectory(std::string dir)
        {
            ZipCreator creator;
            creator.inputFolder = dir;
            creator.createFromDir = true;
            creator.createFromMemEntries = false;
            return creator;
        }
    };

    vector<ZipEntry> GetZipEntries(string file);
    vector<ZipEntry> GetZipEntries(vector<uint8_t> &buffer);
    vector<uint8_t> ExtractEntryToMemory(std::string file, string entryName);
    vector<uint8_t> ExtractEntryToMemory(vector<uint8_t> &zipbuffer, string entryName);
    bool ExtractEntryToFile(std::string file, string outFile, string entryName);
    bool ExtractEntryToFile(vector<uint8_t> &zipbuffer, string outFile, string entryName);
    bool ExtractToFolder(std::string file, string destFolder);
    bool ExtractToFolder(vector<uint8_t> &zipbuffer, string destFolder);
}
// void extrct()
// {

//     char *outbuf = NULL;
//     size_t outbufsize = 0;

//     const char *inbuf = "Append some data here...\0";
//     struct zip_t *zip = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
//     {
//         zip_entry_open(zip, "foo-1.txt");
//         {
//             zip_entry_write(zip, inbuf, strlen(inbuf));
//         }
//         zip_entry_close(zip);

//         /* copy compressed stream into outbuf */
//         zip_stream_copy(zip, (void **)&outbuf, &outbufsize);
//     }
//     zip_stream_close(zip);

//     free(outbuf);
// }