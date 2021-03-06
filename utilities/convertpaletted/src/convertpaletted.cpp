#include <iostream>
#include <string>
#include <memory>

#include <deque>
#include <SDL.h>
#include <SDL_image.h>
#if defined(_WIN32)
#include <Windows.h>
#define REGEX_DIRSEP "[/\\\\]"
#else
#include <dirent.h>
#include <sys/stat.h>
#define REGEX_DIRSEP "/"
#endif

/**********************************************************************************************************/

struct FileSystem
{
    static std::string getDirectorySeparator()
    {
#if defined(_WIN32)
        return "\\";
#else
        return "/";
#endif
    }
    /**
     * @brief
     *  Get the current working directory of this process.
     * @return
     *  the current working directory of this process
     * @throw std::runtime_error
     *  if the current working directory can not be obtained
     * @throw std::bad_alloc
     *  if an out of memory situation occurred
     */
    static std::string getWorkingDirectory()
    {
#if _WIN32
        auto length = GetCurrentDirectory(0, NULL);
        if (!length)
        {
            throw std::runtime_error("unable to obtain working directory");
        }
        auto buffer = std::make_unique<char[]>(length + 1);
        length = GetCurrentDirectory(length + 1, buffer.get());
        if (!length)
        {
            throw std::runtime_error("unable to obtain working directory");
        }
        return std::string(buffer.get());
#else
        // not really needed
        return ".";
#endif
    }
    static std::string sanitize(const std::string& pathName)
    {
#ifdef _WIN32
        char buffer[MAX_PATH+1];
        auto result = GetFullPathName(pathName.c_str(),
                                      MAX_PATH+1,
                                      buffer,
                                      NULL);
        if (!result)
        {
            throw std::runtime_error("unable to sanitize path name");
        }
        return std::string(buffer);
#else
        // not really needed
        return pathName;
#endif
    }
};

/**********************************************************************************************************/

enum class PathStat {
    FILE,
    DIRECTORY,
    OTHER,
    FAILURE
};

void convert(const std::string &fileName);
void recurDir(const std::string &dirName, std::deque<std::string> &queue);
PathStat stat(const std::string &pathName);

/// Filter: accept if string ends with ".bmp".
struct bitmap_filter : public std::unary_function<std::string, bool> {

    bitmap_filter() {}
    bool operator()(const std::string &s) const {
        return s.rfind(".bmp") != std::string::npos;
    }
};

#include "filters.hpp"
#include <sstream>

namespace ConvertPaletted {
	void run(int argc, char **argv) {
		if (argc == 1) {
			std::ostringstream os;
			os << "wrong number of arguments" << std::endl;
			throw std::runtime_error(os.str());
		}

		SDL_Init(0);
		IMG_Init(IMG_INIT_PNG);
		std::deque<std::string> queue;
		RegexFilter filter("^(?:.*" REGEX_DIRSEP ")?(?:tris|tile)[0-9]+\\.bmp$");
		/// @todo Do *not* assume the path is relative. Ensure that it is absolute by a system function.
		for (int i = 1; i < argc; i++) {
			queue.emplace_back(FileSystem::sanitize(argv[i]));
		}
		while (!queue.empty()) {
			std::string path = queue[0];
			queue.pop_front();
			switch (stat(path)) {
			case PathStat::FILE:
				if (filter(path)) {
					convert(path);
				}
				break;
			case PathStat::DIRECTORY:
				recurDir(path, queue);
				break;
			case PathStat::FAILURE:
				break; // stat complains
			default:
				{
					std::ostringstream os;
					os << "skipping '" << path << "' - not a file or directory" << std::endl;
					std::cerr << os.str();
				}
			}
		}
		IMG_Quit();
		SDL_Quit();
	}
}

#if defined(_WIN32)
PathStat stat(const std::string &pathName) {
    DWORD attributes = GetFileAttributes(pathName.c_str());
    if (0xFFFFFFFF == attributes) {
        std::cerr << pathName << ": " << "GetFileAttributes failed" << std::endl;
        return PathStat::FAILURE;
    }
    DWORD rejectedMask = (FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VIRTUAL);
    if (attributes & rejectedMask) {
        return PathStat::OTHER;
    }
    else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        return PathStat::DIRECTORY;
    }
    // It is not rejected and it is not a directory.
    if (attributes == FILE_ATTRIBUTE_NORMAL || attributes & FILE_ATTRIBUTE_ARCHIVE) {
        return PathStat::FILE;
    } else {
        return PathStat::OTHER;
    }
}
#else
PathStat stat(const std::string &pathName) {
    struct stat out;
    int success = lstat(pathName.c_str(), &out);
    if (success == -1) {
        std::cerr << pathName << ": " << strerror(errno) << std::endl;
        return PathStat::FAILURE;
    }
    if (S_ISREG(out.st_mode)) return PathStat::FILE;
    if (S_ISDIR(out.st_mode)) return PathStat::DIRECTORY;
    return PathStat::OTHER;
}
#endif

#if defined(_WIN32)
void recurDir(const std::string &pathName, std::deque<std::string> &queue) {
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile((pathName + "\\*").c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        std::cerr << pathName << ": " << "FindFirstFile failed" << std::endl;
        return;
    }
    do {
        if (ffd.cFileName[0] == '.')
        {
            if ((ffd.cFileName[1] == '.' && ffd.cFileName[2] == '\0') || ffd.cFileName[1] == '\0')
            {
                continue;
            }
        }
        std::string path = pathName + "\\" + ffd.cFileName;
        queue.push_back(path);
    } while (FindNextFile(hFind, &ffd) != 0);
    FindClose(hFind);
}
#else
void recurDir(const std::string &pathName, std::deque<std::string> &queue) {
    DIR *dir = opendir(pathName.c_str());
    if (!dir) {
        std::cerr << pathName << ": " << strerror(errno) << std::endl;
        return;
    }
    while (dirent *aFile = readdir(dir)) {
        if (aFile->d_name[0] == '.') continue;
        std::string path = pathName + "/" + aFile->d_name;
        queue.push_back(path);
    }
    closedir(dir);
}
#endif

/// Perform a dry-run, just print the files which would have been converted.
static bool g_dryrun = false;



void convert(const std::string &fileName) {
    if (fileName.rfind(".bmp") == std::string::npos) return;
    std::string out = fileName;
    size_t extPos = out.rfind('.');
    if (extPos != std::string::npos) out = out.substr(0, extPos);
    out += ".png";
    if (!g_dryrun) {
        SDL_Surface *surf = IMG_Load(fileName.c_str());
        if (!surf) {
            std::cerr << std::string("Couldn't open ") << fileName << ": " << IMG_GetError() << std::endl;
            return;
        }
        uint8_t r, g, b, a;
        if (surf->format->palette) {
            SDL_GetRGBA(0, surf->format, &r, &g, &b, &a);
            std::cout << fileName << ": converting [" << uint16_t(r) << ", " << uint16_t(g) << ", " <<
                uint16_t(b) << ", " << uint16_t(a) << "] to alpha" << std::endl;
            SDL_SetColorKey(surf, SDL_TRUE, 0);
        }
        else {
            std::cout << fileName << ": no palette" << std::endl;
        }
        SDL_Surface *other = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(surf);
        if (!other) {
            std::cerr << "Couldn't convert " << fileName << ": " << SDL_GetError() << std::endl;
            return;
        }
        uint32_t *pixels = (uint32_t *)other->pixels;
        for (int i = 0; i < other->w * other->h; i++, pixels++) {
            SDL_GetRGBA(*pixels, other->format, &r, &g, &b, &a);
            float aFloat = a / 255;
            r *= aFloat; g *= aFloat; b *= aFloat;
            *pixels = SDL_MapRGBA(other->format, r, g, b, a);
        }
        if (IMG_SavePNG(other, out.c_str()) == -1)
            std::cerr << "Couldn't save " << out << ":" << IMG_GetError() << std::endl;
        SDL_FreeSurface(other);
    }
    else
    {
        std::cout << "converting " << fileName << " to " << out << std::endl;
    }
}
