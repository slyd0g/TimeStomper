#include "pch.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <stdlib.h>
using namespace std;

//https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
WORD devurandom(int lower, int upper)
{
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(lower, upper); // define the range
	return distr(eng); // generate numbers
}

//Generate random SYSTEMTIME
SYSTEMTIME SYSTEMTIME_rand()
{
	SYSTEMTIME st = { devurandom(1601,30827), devurandom(1,12), NULL, devurandom(1,31), devurandom(0,23), devurandom(0,59), devurandom(0,59), devurandom(1,999)};
	return st;
}

//Copy SYSTEMTIME of specified file
DWORD get_file_time(string path, SYSTEMTIME m_st, SYSTEMTIME a_st, SYSTEMTIME c_st)
{
	HANDLE hHandle;
	FILETIME m_ft = {};
	FILETIME a_ft = {};
	FILETIME c_ft = {};

	//Get file handle
	hHandle = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		cout << "CreateFileA() fail on " << path << "." <<" SYSTEM ERROR: " << GetLastError() << "\n";
		return 1;
	}
		
	GetFileTime(hHandle, &c_ft, &a_ft, &m_ft);
	FileTimeToSystemTime(&m_ft, &m_st);
	FileTimeToSystemTime(&a_ft, &a_st);
	FileTimeToSystemTime(&c_ft, &c_st);
	
	return 0;
}

SYSTEMTIME string_to_SYSTEMTIME(string date, string time)
{
	std::stringstream date_stream(date);
	std::stringstream time_stream(time);
	std::string date_segment;
	std::string time_segment;
	std::vector<std::string> date_list;
	std::vector<std::string> time_list;

	while (std::getline(date_stream, date_segment, '-'))
	{
		date_list.push_back(date_segment);
	}
	while (std::getline(time_stream, time_segment, ':'))
	{
		time_list.push_back(time_segment);
	}

	int month = atoi(date_list[0].c_str());
	int day = atoi(date_list[1].c_str());
	int year = atoi(date_list[2].c_str());
	int hour = atoi(time_list[0].c_str());
	int minute = atoi(time_list[1].c_str());
	int second = atoi(time_list[2].c_str());
	int millisecond = atoi(time_list[3].c_str());

	SYSTEMTIME st = { year, month, NULL, day, hour, minute, second, millisecond };
	return st;
}

//0 for file, 1 for dir, 2 for wtf
DWORD file_or_dir(string path)
{
	char *pathptr = &path[0u];
	struct stat s;
	if (stat(pathptr, &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
		{
			return 1;
		}
		else if (s.st_mode & S_IFREG)
		{
			return 0;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		return 2;
	}
}

void timestomp(string path, SYSTEMTIME m_st, SYSTEMTIME a_st, SYSTEMTIME c_st, int m_flag, int a_flag, int c_flag)
{
	HANDLE hHandle;
	FILETIME m_ft = {};
	FILETIME a_ft = {};
	FILETIME c_ft = {};
	
	//Convert SYSTEMTIME to FILETIME, if time isn't specified keep current filetime
	if (m_flag == 1)
		SystemTimeToFileTime(&m_st, &m_ft);
	else
	{
		m_ft.dwLowDateTime = 0;
		m_ft.dwHighDateTime = 0;
	}
	if (a_flag == 1)
		SystemTimeToFileTime(&a_st, &a_ft);
	else
	{
		a_ft.dwLowDateTime = 0;
		a_ft.dwHighDateTime = 0;
	}
	if (c_flag == 1)
		SystemTimeToFileTime(&c_st, &c_ft);
	else
	{
		c_ft.dwLowDateTime = 0;
		c_ft.dwHighDateTime = 0;
	}
	
	//Get file handle
	hHandle = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		cout << "CreateFileA() fail on " << path << "." << " SYSTEM ERROR: " << GetLastError() << "\n";
	
	//Modify time
	if (SetFileTime(hHandle, &c_ft, &a_ft, &m_ft) == NULL)
		wcout << "SetFileTime() fail. SYSTEM ERROR: " << GetLastError() << "\n";
	else
		cout << "SetFileTime() success on " << path << endl; //DEBUG ONLY
}

//https://github.com/rapid7/meterpreter/blob/master/source/extensions/priv/server/timestomp.c#L325
DWORD timestomp_recursive(string path, SYSTEMTIME m_st, SYSTEMTIME a_st, SYSTEMTIME c_st, int m_flag, int a_flag, int c_flag, int r_flag)
{
	string path_patch = path + "\\*";
	WIN32_FIND_DATAA w32fd;
	HANDLE hSearch = FindFirstFileA(path_patch.c_str(), &w32fd);

	//Check search handle created successfully
	if (hSearch != INVALID_HANDLE_VALUE)
		;
	else if (hSearch == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == 5) //Skip access denied
		{
			return 1;
		}
		cout << "Invalid handle value for path: " << path_patch << endl;
		wcout << "System Error Code: " << GetLastError() << "\n";
		return 0;
	}

	//If (directory), else (file)
	if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
	{
		if ((string(w32fd.cFileName) != ".") && (string(w32fd.cFileName) != "..")) 
		{
			string current_file = path + "\\" + w32fd.cFileName;
			timestomp(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag);
			//Recurse through all sub-folders
			if (r_flag)
			{
				if (timestomp_recursive(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag, r_flag) == 0)
					return 0;
			}
		}
	}
	else
	{
		string current_file = path + "\\" + w32fd.cFileName;
		timestomp(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag);
	}

	//If (directory), else (file)
	while (FindNextFileA(hSearch, &w32fd) != 0)
	{
		if(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((string(w32fd.cFileName) != ".") && (string(w32fd.cFileName) != "..")) 
			{
				string current_file = path + "\\" + w32fd.cFileName;
				timestomp(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag);
				//Recurse through all sub-folders
				if (r_flag)
				{
					if (timestomp_recursive(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag, r_flag) == 0)
						return 0;
				}
			}
		}
		else
		{
			string current_file = path + "\\" + w32fd.cFileName;
			timestomp(current_file, m_st, a_st, c_st, m_flag, a_flag, c_flag);
		}
	}
	return 1;
}

void help()
{
	cout << "                 <TimeStomper Usage>" << endl << endl;
	cout << "NOTE: All times are interpreted as UTC." << endl;
	cout << "      Milliseconds are always random." << endl;
	cout << "      Use \"r\" instead of <date> <time> to set random time (see example)." << endl << endl;
	cout << "   -m <date> <time> (set last write time)" << endl;
	cout << "   -a <date> <time> (set last access time)" << endl;
	cout << "   -c <date> <time> (set creation time)" << endl;
	cout << "   -z <date> <time> (set all times)" << endl;
	cout << "   -p <full-path> (file or folder to set time)" << endl;
	cout << "   -p2 <full-path> (file or folder to copy time from)" << endl;
	cout << "   -r (recurse through all subfolders and files)" << endl;
	cout << "   -h (print this menu)" << endl << endl;
	cout << "Example:" << endl;
	cout <<	"   -m r -p C:\\full\\path (set last modified time at C:\\full\\path to random date and time)" << endl;
	cout << "   -z 10-20-1994 14:2:01 -p C:\\full\\path -r (recursively set MAC time for files under C:\\full\\path to October 20, 1994 2:02:01 PM)" << endl;
	cout << "   -p C:\\full\\path -p2 C:\\full\\path2 (copies MAC time from C:\\full\\path2 to C:\\full\\path)" << endl;
}

int main(int argc, char **argv)
{
	
	string m_date;
	string m_time;
	string a_date;
	string a_time;
	string c_date;
	string c_time;
	string path;
	string path2;
	SYSTEMTIME m_st;
	SYSTEMTIME a_st;
	SYSTEMTIME c_st;
	int m_flag = NULL;
	int a_flag = NULL;
	int c_flag = NULL;
	int r_flag = NULL;
	int z_flag = NULL;

	//Parse flags
	for (int i = 0; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-h") || (argc == 1))
		{
			help();
			return 0;
		}
		if (!strcmp(argv[i], "-m") && (z_flag == NULL))
		{
			string m_date_temp(argv[i + 1]);
			if (m_date_temp == "r")
			{
				m_st = SYSTEMTIME_rand();
				m_flag = 1;
			}
			else
			{
				string m_time_temp(argv[i + 2]);
				m_date = m_date_temp;
				m_time = m_time_temp + ":" + to_string(devurandom(1, 999)); //randomize milliseconds
				m_st = string_to_SYSTEMTIME(m_date, m_time);
				m_flag = 1;
			}
		}
		if (!strcmp(argv[i], "-a") && (z_flag == NULL))
		{
			string a_date_temp(argv[i + 1]);
			if (a_date_temp == "r")
			{
				a_st = SYSTEMTIME_rand();
				a_flag = 1;
			}
			else
			{
				string a_time_temp(argv[i + 2]);
				a_date = a_date_temp;
				a_time = a_time_temp + ":" + to_string(devurandom(1, 999)); //randomize milliseconds
				a_st = string_to_SYSTEMTIME(a_date, a_time);
				a_flag = 1;
			}
		}
		if (!strcmp(argv[i], "-c") && (z_flag == NULL))
		{
			string c_date_temp(argv[i + 1]);
			if (c_date_temp == "r")
			{
				c_st = SYSTEMTIME_rand();
				c_flag = 1;
			}
			else
			{
				string c_time_temp(argv[i + 2]);
				c_date = c_date_temp;
				c_time = c_time_temp + ":" + to_string(devurandom(1, 999)); //randomize milliseconds
				c_st = string_to_SYSTEMTIME(c_date, c_time);
				c_flag = 1;
			}
		}
		if (!strcmp(argv[i], "-z"))
		{
			string m_date_temp(argv[i + 1]);
			if (m_date_temp == "r")
			{
				m_st = SYSTEMTIME_rand();
				a_st = SYSTEMTIME_rand();
				c_st = SYSTEMTIME_rand();
				m_flag = 1;
				a_flag = 1;
				c_flag = 1;
				z_flag = 1;
			}
			else
			{
				string m_time_temp(argv[i + 2]);
				m_date = m_date_temp;
				m_time = m_time_temp + ":" + to_string(devurandom(1, 999));
				m_st = string_to_SYSTEMTIME(m_date, m_time);
				a_date = m_date_temp;
				a_time = m_time_temp + ":" + to_string(devurandom(1, 999));
				a_st = string_to_SYSTEMTIME(m_date, m_time);
				c_date = m_date_temp;
				c_time = m_time_temp + ":" + to_string(devurandom(1, 999));
				c_st = string_to_SYSTEMTIME(m_date, m_time);
				m_flag = 1;
				a_flag = 1;
				c_flag = 1;
				z_flag = 1;
			}
		}
		if (!strcmp(argv[i], "-p"))
		{
			string path_temp(argv[i + 1]);
			path = path_temp;
			cout << "Modifying time for " << path << endl;
		}
		if (!strcmp(argv[i], "-p2"))
		{
			string path2_temp(argv[i + 1]);
			path2 = path2_temp;
			cout << "Copying time from " << path2 << endl;
			
			HANDLE hHandle;
			FILETIME m_ft = {};
			FILETIME a_ft = {};
			FILETIME c_ft = {};
			m_flag = 1;
			a_flag = 1;
			c_flag = 1;

			//Get file handle
			hHandle = CreateFileA(path2.c_str(), FILE_READ_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if (hHandle == INVALID_HANDLE_VALUE)
			{
				cout << "CreateFileA() fail on " << path2 << "." << " SYSTEM ERROR: " << GetLastError() << "\n";
				return 0;
			}

			GetFileTime(hHandle, &c_ft, &a_ft, &m_ft);
			FileTimeToSystemTime(&m_ft, &m_st);
			FileTimeToSystemTime(&a_ft, &a_st);
			FileTimeToSystemTime(&c_ft, &c_st);
		}
		if (!strcmp(argv[i], "-r"))
		{
			r_flag = 1;
		}
	}
	
	//File or directory?
	DWORD path_flag = file_or_dir(path);
	if (path_flag == 0)
		timestomp(path, m_st, a_st, c_st, m_flag, a_flag, c_flag);
	else if (path_flag == 1)
	{
		timestomp(path, m_st, a_st, c_st, m_flag, a_flag, c_flag);
		timestomp_recursive(path, m_st, a_st, c_st, m_flag, a_flag, c_flag, r_flag);
	}
	else
	{
		cout << "Could not open a handle to specified path. Typo? Quote your path if there are spaces?" << endl;
		return 0;
	}

	return 0;
}
