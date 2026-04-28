#include <windows.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "FileInfo.h"

using namespace std;

static std::string BuildDate;


void PrintFileTime(const FILETIME& ft, const char* label) {
    if (ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0) {
        cout << label << ": N/A" << endl;
        return;
    }

    // Конвертируем FILETIME в SYSTEMTIME
    FILETIME localFt;
    SYSTEMTIME st;

    FileTimeToLocalFileTime(&ft, &localFt);
    FileTimeToSystemTime(&localFt, &st);

    stringstream ss;
    ss << setfill('0') << setw(2) << st.wDay << "."
        << setw(2) << st.wMonth << "."
        << st.wYear << " "
        << setw(2) << st.wHour << ":"
        << setw(2) << st.wMinute << ":"
        << setw(2) << st.wSecond << endl;

    // Выводим дату и время
    cout << label << ": ";
    cout << ss.str();
    if (label == "Modified")
    {
        BuildDate = ss.str();
    }

}

void GetExeFileInfo(const char* filename) {
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;

    if (!GetFileAttributesExA(filename, GetFileExInfoStandard, &fileAttr)) {
        DWORD error = GetLastError();
        cout << "Error opening file: " << error << endl;
        return;
    }

    cout << "=== File Information: " << filename << " ===" << endl;
    cout << "File size: " << fileAttr.nFileSizeLow << " bytes";

    if (fileAttr.nFileSizeHigh > 0) {
        ULONGLONG fullSize = ((ULONGLONG)fileAttr.nFileSizeHigh << 32) | fileAttr.nFileSizeLow;
        cout << " (" << fullSize << " bytes)" << endl;
    }
    else {
        cout << endl;
    }

    // Время создания
    PrintFileTime(fileAttr.ftCreationTime, "Created");

    // Время последнего доступа
    PrintFileTime(fileAttr.ftLastAccessTime, "Accessed");

    // Время последнего изменения
    PrintFileTime(fileAttr.ftLastWriteTime, "Modified");

    // Атрибуты файла
    cout << "\nAttributes: ";
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout << "ARCHIVE ";
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout << "READONLY ";
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout << "HIDDEN ";
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout << "SYSTEM ";
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) cout << "DIRECTORY ";
    cout << endl;
}


std::string GetBuildDate()
{
    return BuildDate;
}
