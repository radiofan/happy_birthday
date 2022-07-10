#pragma once

#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "Shlwapi.lib")
#include <shlwapi.h>
//#include <strsafe.h>
//#include <commctrl.h>
#include <vector>
#include <string>
#include <tchar.h>
#ifdef _UNICODE
	typedef std::wstring String;
	#define str_to_tstr(X) string_to_wstring(X)
	#define toString to_wstring

#else
	typedef std::string String;
	#define str_to_tstr(X) X
	#define toString to_string
#endif

#define WM_SHELLNOTIFY (WM_APP + 1)
#define DEFAULT_CLOSE_AFTER_SEC 5
#define STR_(X) #X
#define STR(X) STR_(X)

#define szInfoTitle_LENGTH 64
#define szInfo_LENGTH 256
#define szTip_LENGTH 128

#define nano100SecInWeek (__int64)10000000*60*60*24*7
#define nano100SecInDay  (__int64)10000000*60*60*24
#define nano100SecInHour (__int64)10000000*60*60
#define nano100SecInMin  (__int64)10000000*60
#define nano100SecInSec  (__int64)10000000

//разделитель данных в строке в файле
#define DELIMITER_CHAR '\t'

//интервал между отображением разных типов баллонов
#define BALLON_TYPES_INTERVAL_SEC 5

//идентификаторы таймеров
#define CLOSE_TIMER 1
#define BALLON_TYPES_INTERVAL_TIMER 2

//идентификаторы меню
#define CM_EXIT 15
#define CM_EXEC 14


std::wstring string_to_wstring(const std::string& str);


#include "BirthdayList.h"
#include "BirthdaysClass.h"