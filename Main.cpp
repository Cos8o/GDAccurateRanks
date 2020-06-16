#include <windows.h>
#include <Psapi.h>

#include <vector>
#include <string>
#include <iostream>

static auto constexpr MAX_UNICODE_PATH = 32767u;

static uintptr_t constexpr PATCH_ADDRESS = 0x29B130;

//Base64 version of "http://c8o.altervista.org/database/getAccScores.php"
static std::vector<uint8_t> const PATCH_BYTES =
{
	0x61, 0x48, 0x52, 0x30, 0x63, 0x44, 0x6F, 0x76,
	0x4C, 0x32, 0x4D, 0x34, 0x62, 0x79, 0x35, 0x68,
	0x62, 0x48, 0x52, 0x6C, 0x63, 0x6E, 0x5A, 0x70,
	0x63, 0x33, 0x52, 0x68, 0x4C, 0x6D, 0x39, 0x79,
	0x5A, 0x79, 0x39, 0x6B, 0x59, 0x58, 0x52, 0x68,
	0x59, 0x6D, 0x46, 0x7A, 0x5A, 0x53, 0x39, 0x6E,
	0x5A, 0x58, 0x52, 0x42, 0x59, 0x32, 0x4E, 0x54,
	0x59, 0x32, 0x39, 0x79, 0x5A, 0x58, 0x4D, 0x75,
	0x63, 0x47, 0x68, 0x77
};

//Error message

void logError(std::string const& s, bool gle)
{
	std::string msg("ERROR: " + s);

	if (gle)
		msg += " (" + std::to_string(GetLastError()) + ')';

	std::cout << msg << '\n';
	system("pause>nul");
}

//Mapping utilities

HANDLE createMap(std::wstring const& s)
{
	HANDLE map = NULL;

	auto file = CreateFileW(
		s.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		map = CreateFileMappingW(
			file,
			NULL,
			PAGE_READWRITE,
			NULL,
			NULL,
			NULL);

		CloseHandle(file);
	}

	return map;
}

uintptr_t initMap(HANDLE const map)
{
	if (map)
	{
		return reinterpret_cast<uintptr_t>(
			MapViewOfFile(
				map,
				FILE_MAP_READ | FILE_MAP_WRITE,
				NULL,
				NULL,
				NULL));
	}

	return NULL;
}

bool freeMap(HANDLE map, uintptr_t base)
{
	return UnmapViewOfFile(reinterpret_cast<LPVOID>(base)) &&
		CloseHandle(map);
}

//Find GD path from running process

void getProcessPath(std::wstring& path)
{
	DWORD id = 0, retCode = 0;
	auto p = new wchar_t[MAX_UNICODE_PATH];

	auto window = FindWindowW(NULL, L"Geometry Dash");

	std::cout << "Waiting for Geometry Dash...\n";

	while (!window)
	{
		Sleep(250);
		window = FindWindowW(NULL, L"Geometry Dash");
	}

	GetWindowThreadProcessId(window, &id);

	auto process = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE,
		FALSE,
		id);

	if (process != INVALID_HANDLE_VALUE)
	{
		auto len = GetModuleFileNameExW(
			process,
			NULL,
			p,
			MAX_UNICODE_PATH);

		if (len)
		{
			path = std::wstring(
				p,
				p + len);

			std::cout << "Close the game to continue.\n";

			WaitForSingleObject(process, INFINITE);
		}
	}

	delete[] p;
}

//Main

int wmain(
	int const argc,
	wchar_t const* const* argv)
{
	std::wstring filePath;

	SetConsoleTitleA("Geometry Dash Accurate Ranks Hack");

	system("cls");

	std::cout << "Geometry Dash Accurate Ranks Hack\n";
	std::cout << "By XShadowWizardX, GDColon, Cos8o\n";

	if (argc > 1)
	{
		auto len = wcslen(argv[1]);
		filePath = std::wstring(argv[1], argv[1] + len);
	}
	else
	{
		getProcessPath(filePath);
	}

	if (!filePath.size())
	{
		logError("File not found.", false);
		return 1;
	}

	//Map file

	auto map = createMap(filePath);
	auto base = initMap(map);

	if (!map || !base)
	{
		logError("Could not map file in memory.", true);
		return 1;
	}

	//Apply patch

	std::cout << "Patching..." << '\n';

	auto address = base + PATCH_ADDRESS;

	memcpy(
		reinterpret_cast<void*>(address),
		PATCH_BYTES.data(),
		PATCH_BYTES.size());

	if (!FlushViewOfFile(
		reinterpret_cast<LPVOID>(address),
		PATCH_BYTES.size()))
	{
		logError("Patch failed.", true);
		return 1;
	}

	//Cleanup

	if (!freeMap(map, base))
	{
		logError("Patch failed.", true);
		return 1;
	}

	std::cout << "Patch applied!\n";
	std::cout << "Press any button to exit." << std::endl;
	system("pause>nul");

	return 0;
}
