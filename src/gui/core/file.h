#ifndef __FILE_H__
#define __FILE_H__
#include <string>
#include <vector>
namespace cdroid{
class File {
public:
#if defined(__linux__)||defined(__unix__)||defined(__APPLE__)
    static constexpr char separatorChar = '/';//fs.getSeparator();
    static constexpr const char*const separator = "/";
    static constexpr char pathSeparatorChar =':';
    static constexpr const char*const pathSeparator = ":";
#else
    static constexpr char separatorChar = '\\';
    static constexpr const char*const separator = "\\";
    static constexpr char pathSeparatorChar =';';
    static constexpr const char*const pathSeparator = ";";
#endif    
public:
    // Constructors
    File(const std::string& pathname);
    File(const std::string& parent, const std::string& child);
    File(const File* parent, const std::string& child);
    static std::string getCurrentDirectory();

    // Methods
    std::string getName() const;
    std::string getParent() const;
    std::string getPath() const;
    std::string getAbsolutePath() const;
    std::string getCanonicalPath() const;
    bool exists() const;
    bool isDirectory() const;
    bool isFile() const;
    bool canRead() const;
    bool canWrite() const;
    bool canExecute() const;
    bool createNewFile();
    bool deleteFile();
    std::vector<File> listFiles() const;
    bool mkdir();
    bool mkdirs();
    bool renameTo(const File& dest);
    long length() const;
private:
    std::string path;
    std::string parent;
    std::string name;
    void initialize(const std::string& pathname);
};

}
#endif
