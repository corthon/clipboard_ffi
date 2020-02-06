#include <iostream>
#include <string>
#include <map>
#include <set>

#include <windows.h>

#include "ErrorRecord.hpp"

extern "C"
{
    void
    print_clipboard_file(
        PCWCHAR filename,
        SIZE_T len
    );
}


[[noreturn]] void
Usage()
{
    wprintf_s(L"Usage: clipboard <command> [options...]\n");
    wprintf_s(L"\n");
    wprintf_s(L"paste [-file <filename>]\n");
    wprintf_s(L"    Prints the current contents of the clipboard.\n");
    wprintf_s(L"    If -file, prints the contents to the specified file.\n");
    wprintf_s(L"\n");
    wprintf_s(L"copy -file <filename>\n");
    wprintf_s(L"    Copies the contents of <filename> to the clipboard as text.\n");
    wprintf_s(L"\n");
    wprintf_s(L"copy -text \"<text>\"\n");
    wprintf_s(L"    Copies the provided string to the clipboard as text.\n");
    wprintf_s(L"\n");
    wprintf_s(L"log -file <filename>\n");
    wprintf_s(L"    While clipboard.exe is running, appends anything copied to\n");
    wprintf_s(L"    the clipboard to <filename>\n");
    wprintf_s(L"\n");
    exit(1);
}

const std::set<std::wstring> c_FlagsWithArguments{ L"-file", L"-text" };

void
ReportError(ErrorRecord Rec)
{
    wchar_t buffer[256]{};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr,
                   Rec.ErrorCode,
                   0,
                   buffer,
                   _countof(buffer),
                   nullptr);
    wprintf_s(L"%ls:%d: %ls: %ls\n", Rec.File, Rec.LineNumber, Rec.Message, buffer);
}

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

#define ReportErr( Msg, Code) { \
    ErrorRecord e{}; \
    e.Message = Msg; \
    e.ErrorCode = Code; \
    e.LineNumber = __LINE__; \
    e.File = WFILE; \
    ReportError(e); \
}

void
PrintClipboardText()
{
    auto success = OpenClipboard(nullptr);
    if (!success)
    {
       ReportErr(L"Failed to open clipboard", GetLastError());
       return;
    }

    auto hglb = GetClipboardData(CF_UNICODETEXT);
    if (hglb != NULL)
    {
        LPCWSTR text = (LPCWSTR)GlobalLock(hglb);
        if (text != NULL)
        {
            wprintf_s(L"Clipboard data:\n\n%wS\n", text);
            GlobalUnlock(hglb);
        }
    }
    else
    {
        ReportErr(L"No text on clipboard", GetLastError());
        return;

    }
    CloseClipboard();

    return;
}

/// Checks to see if a filename was passed in as a flag,
/// extracts it, and checks for -A for ANSI format.
std::pair<std::wstring, bool>
ParseFileFlag(
    std::map<std::wstring, std::wstring>& flags
)
{
    // Check if a filename was passed in.
    std::wstring filename;
    bool useANSItext = false;
    if (flags.find(L"-file") != flags.end())
    {
        filename = flags[L"-file"];
        flags.erase(L"-file");
        useANSItext = flags.erase(L"-A") > 0;
    }

    return std::make_pair(filename, useANSItext);
}

int wmain(int argc, const wchar_t** argv)
{
    int argi = 0;
    std::map<std::wstring, std::wstring> flags;

    // Get the next arg or print usage and exit
    // if there are no more arguments.
    auto nextArg = [&]()
    {
        if (argi == argc)
        {
            Usage();
        }

        return argv[argi++];
    };

    // Called when the command line is expected to be fully parsed. Exits
    // with usage if there are any more arguments or uninterpreted flags.
    auto doneArgs = [&]()
    {
        if (argi < argc || !flags.empty())
        {
            Usage();
        }
    };

    // Get the command
    nextArg();
    auto command = nextArg();

    // Parse the flags
    while (argi < argc && argv[argi][0] == '-')
    {
        std::wstring flag{nextArg()};
        PCWSTR value{L""};
        if (c_FlagsWithArguments.find(flag) != c_FlagsWithArguments.end())
        {
            value = nextArg();
        }

        if (!flags.try_emplace(std::move(flag), value).second)
        {
            Usage();
        }
    }

    // Interpret the flags and execute the command
    if (!_wcsicmp(command, L"paste"))
    {
        // Check if a filename was specified.
        auto [filename, useANSItext] = ParseFileFlag(flags);

        doneArgs();

        // If no filename was passed in, print to console.
        if (filename.empty())
        {
            PrintClipboardText();
        }
        else
        {
            // Print to a file
            print_clipboard_file(filename.c_str(), filename.length());
        }
    }
    else if (!_wcsicmp(command, L"copy"))
    {
        auto [filename, useANSItext] = ParseFileFlag(flags);
        if (!filename.empty())
        {
            doneArgs();
            // Copy the contents of filename to the clipboard
        }
        else
        {
            auto text = flags[L"-text"];
            flags.erase(L"-text");
            doneArgs();

            // Copy text to the clipboard
        }

    }
    else if (!_wcsicmp(command, L"log"))
    {
        auto [filename, useANSItext] = ParseFileFlag(flags);
        doneArgs();

        // Log clipboard activity to the file
    }
    else {
        Usage();
        return 1;
    }

    return 0;
}
