#include "stdafx.h"

std::wstring string_to_wstring(const std::string& str){
	UINT32 wstr_len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	if(!wstr_len)
		return std::wstring();
	HANDLE h_process_heap = GetProcessHeap();
	LPWSTR wstr = (LPWSTR)HeapAlloc(h_process_heap, 0, wstr_len * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, wstr_len);
	std::wstring ret(wstr);
	HeapFree(h_process_heap, 0, wstr);
	return ret;
}



LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);


class App{
public:
	//f0ad928e-9908-497f-8f95-bc42c9e1805c
	const GUID appGUID = {0xf0ad928e, 0x9908, 0x497f, {0x8f, 0x95, 0xbc, 0x42, 0xc9, 0xe1, 0x80, 0x5c}};
private:
	NOTIFYICONDATA icon_struct;
	WNDCLASSEX main_win_c;
	HWND window;
	HINSTANCE hInstance;
	HMENU context_menu;
	SYSTEMTIME current_time;


	BirthdaysClass birthdays_list;
	/**
	* 
	*/
	UINT8 birthday_type_print;


	//параметры задаются параметрами запуска
	bool args_parse_result;

	/**
	* вывести справку
	* --help -h /? -?
	*/
	bool is_help;
	/**
	* закрыть программу после отображения всех уведомлений через указанное количество сек
	* если < 0 программа не закроется
	* /c[ количество ожидания в сек]
	* -c[ количество ожидания в сек]
	* --close[ количество ожидания в сек]
	*/
	int close_after_sec;

public:
	/**
	* главный класс программы, создать и вернуть в главной функции App::start
	* @param hInstance - 
	* @param comand_args - указатель на строку с аргументами
	*/
	App(HINSTANCE hInstance, LPTSTR comand_args)
		: icon_struct({0}),
		main_win_c({0}),
		window(nullptr),
		hInstance(hInstance),
		context_menu(nullptr),
		current_time({0}),

		birthday_type_print(0),

		is_help(false),
		close_after_sec(-1)
	{
		args_parse_result = this->parse_comand_args(comand_args);
		if(!args_parse_result || is_help)
			return;
		GetLocalTime(&current_time);
		window = create_window();
		this->init_icon_struct();
		this->init_context_menu();
	}

	/**
	* Удаляет значок из трея
	* @see App::icon_struct
	*/
	void hide_icon(){
		Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	}

	/**
	* запускает обработку событий
	* или выводит уведомление об ошибке парсинга параметров (args_parse_result == false)
	* @see App::args_parse_result
	* или выводит справку (is_help == true)
	* @see App::is_help
	* @return код завершения работы приложения
	*/
	int start(){
		if(!args_parse_result){
			MessageBox(NULL, TEXT("Ошибка распознвания параметров"), TEXT("Ошибка"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return 1;
		}
		if(is_help){
			MessageBox(
				NULL,
				TEXT("Description\n\n")//todo
				TEXT("app.exe   [/c[ секунды]]   [/?]\n\n")
				TEXT("  /c (-c, --close)\tПроизведет закрытие программы через указанное количество секунд(целое) (по умолчанию ")
				TEXT(STR(DEFAULT_CLOSE_AFTER_SEC))
				TEXT(" сек) после отображения всех уведомлений, если не получает событий\n")
				TEXT("  /? (-?, -h, --help)\tВывод справки\n"),
				TEXT("Справка"),
				MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
			);
			return 0;
		}

		HANDLE h_birthdays_file = open_birthdays_file();
		if(h_birthdays_file != NULL && h_birthdays_file != INVALID_HANDLE_VALUE){
			//FILETIME last_write_time = {0};
			//GetFileTime(h_birthdays_file, NULL, NULL, &last_write_time);
			//todo сравнение с последней датой изменения при последнем парсинге
			//есть ли смысл сохранять кэш если его все равно парсить нужно?
			birthdays_list.parse_data_from_file(h_birthdays_file);

			CloseHandle(h_birthdays_file);
		}

		if(birthdays_list.get_count_birthdays(current_time.wMonth)){
			birthday_type_print = 1;
			SetTimer(window, BALLON_TYPES_INTERVAL_TIMER, BALLON_TYPES_INTERVAL_SEC*1000, NULL);
		}else{
			if(close_after_sec >= 0){
				SetTimer(window, CLOSE_TIMER, close_after_sec*1000, NULL);
			}
		}

		Shell_NotifyIcon(NIM_ADD, &icon_struct);
		Shell_NotifyIcon(NIM_SETVERSION, &icon_struct);

		// Цикл обработки сообщений
		MSG message;
		while(GetMessage(&message, NULL, 0, 0)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return 0;
	}

	/**
	* отображает контекстное меню context_menu в указанных координатах
	*/
	bool show_context_menu(INT16 x, INT16 y){
		SetForegroundWindow(window);
		bool ret = TrackPopupMenu(
			context_menu,
			//TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, 
			TPM_LEFTBUTTON, 
			x, 
			y,
			0,
			window,
			NULL
		);
		return ret;
	}

	/**
	* Обрабатывает выбора элемента контекстного меню
	* @param menu_item_ind - идентификатор элемента меню
	* CM_EXIT - выход из приложения
	* 
	*/
	void context_menu_click(INT16 menu_item_ind){
		switch(menu_item_ind){
			case CM_EXIT:
				DestroyWindow(window);
				break;
			case CM_EXEC:
				ModifyMenu(context_menu, CM_EXEC, MF_BYCOMMAND | MF_UNCHECKED | MFT_STRING, NULL, TEXT("Действие"));
				break;
		}
	}

	void show_birthdays_ballons(){
		//день рождения в этом месяце
		if(birthday_type_print == 1){
			BirthdayList b_list = birthdays_list.get_birthdays(current_time.wMonth, 0);
			lstrcpyn(icon_struct.szInfoTitle, TEXT("В этом месяце день рождения празднуют:"), szInfoTitle_LENGTH);
			String out;
			String birthday_greeting;
			UINT32 len = b_list.size();
			for(UINT32 i=0; i<len; i++){
				birthday_greeting = b_list.get_greeting_with_day_age(i, current_time);
				if(birthday_greeting.size() >= szInfo_LENGTH-1){
					continue;
				}
				if(birthday_greeting.size() + out.size() >= szInfo_LENGTH-2){
					break;
				}
				if(out.size()){
					out += TEXT("\n");
				}
				out += birthday_greeting;
			}

			lstrcpyn(icon_struct.szInfo, out.c_str(), szInfo_LENGTH);
			Shell_NotifyIcon(NIM_MODIFY, &icon_struct);
		}

		KillTimer(window, BALLON_TYPES_INTERVAL_TIMER);
	}

	/**
	* Очищает память
	*/
	~App(){
		if(!args_parse_result || is_help)
			return;
		//очистка памяти
		DestroyIcon(icon_struct.hIcon);
		DestroyIcon(icon_struct.hBalloonIcon);
		DestroyMenu(context_menu);
	}

private:
	/**
	* регистрирует класс окна main_win_c
	* @see App::main_win_c
	* создает окно
	* @return дескриптор созданного окна
	*/
	HWND create_window(){
		//Регистрация класса окна
		main_win_c.cbSize = sizeof(WNDCLASSEX);
		main_win_c.hInstance = hInstance;
		main_win_c.lpszClassName = TEXT("Main");
		main_win_c.lpfnWndProc = WndProc;
		RegisterClassEx(&main_win_c);

		// Создание главного окна
		return CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	}

	/**
	* производит парсинг аргументов командной строки
	* @param comand_args - указатель на строку с аргументами
	* @see App::is_help
	* @see App::close_after_sec
	* @return удалось ли распарсить аргументы
	*/
	bool parse_comand_args(LPTSTR comand_args){
		#ifdef UNICODE
			LPWSTR *arg_list;
			int arg_c;

			arg_list = CommandLineToArgvW(comand_args, &arg_c);
			if(arg_list == NULL){
			  return false;
			}
			for(int i=0; i<arg_c; i++){
				if(
					lstrcmp(arg_list[i], TEXT("-h")) == 0 ||
					lstrcmp(arg_list[i], TEXT("-?")) == 0 ||
					lstrcmp(arg_list[i], TEXT("/?")) == 0 ||
					lstrcmp(arg_list[i], TEXT("--help")) == 0
				){
					is_help = true;
					continue;
				}

				if(
					lstrcmp(arg_list[i], TEXT("-c")) == 0 ||
					lstrcmp(arg_list[i], TEXT("/c")) == 0 ||
					lstrcmp(arg_list[i], TEXT("--close")) == 0
				){
					if(i+1 == arg_c){
						close_after_sec = DEFAULT_CLOSE_AFTER_SEC;
						continue;
					}
					if(StrToIntEx(arg_list[i+1], STIF_DEFAULT, &close_after_sec)){
						i++;
					}else{
						close_after_sec = DEFAULT_CLOSE_AFTER_SEC;
					}
				}
			}

			LocalFree(arg_list);
			return true;
		#else
			#error "parse_comand_args ANSI undefined"
			return false;
		#endif //UNICODE
	}

	/**
	* производит инициализацию структуры icon_struct
	* @see App::icon_struct
	*/
	void init_icon_struct(){
		// Создание значка
		icon_struct.cbSize = sizeof(NOTIFYICONDATA);
		icon_struct.hWnd = window;
		icon_struct.uVersion = NOTIFYICON_VERSION_4;
		icon_struct.uID = 1;

		//используем UUID (только для win7)
		//Обрабатываем события
		//выводим иконку в трей
		//выводим подсказку по наведению
		//стандартный вывод подсказки
		//вывод баллона
		icon_struct.uFlags = NIF_GUID | NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_INFO;

		icon_struct.guidItem = appGUID;

		//если uVersion = 0 или NOTIFYICON_VERSION (не наш случай)
		//Параметр wParam будет определен идентификатором значка, который генерирует сообщение.
		//Параметр lParam "реальный" идентификатор сообщения Windows

		//наш случай
		//LOWORD(lParam) содержит события уведомлений, такие как NIN_BALLOONSHOW, NIN_POPUPOPEN или WM_CONTEXTMENU.
		//HIWORD(lParam) содержит идентификатор значка. Длина идентификаторов значков ограничена 16 битами.
		//GET_X_LPARAM(wParam) возвращает координату привязки X для событий уведомлений NIN_POPUPOPEN, NIN_SELECT, NIN_KEYSELECT и всех сообщений мыши между WM_MOUSEFIRST и WM_MOUSELAST. Если какое-либо из этих сообщений генерируется клавиатурой, параметр wParam устанавливается в верхнем левом углу целевого значка. Для всех остальных сообщений параметр wParam не определен.
		//GET_Y_LPARAM(wParam) возвращает координату привязки Y для событий уведомлений и сообщений, как определено для привязки X.
		icon_struct.uCallbackMessage = WM_SHELLNOTIFY;

		//LoadIconMetric(hInstance, MAKEINTRESOURCE(102), LIM_SMALL, &(icon_struct.hIcon));
		icon_struct.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXICON, SM_CYICON, LR_DEFAULTCOLOR);
		//icon_struct.hIcon = LoadIcon(NULL, IDI_SHIELD);

		/*
		//Строка с нулевым символом в конце. макс 128 символов включая 0
		//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
		lstrcpyn(icon_struct.szTip, TEXT("Test application"), szTip);
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), szInfoTitle_LENGTH);
		lstrcpyn(icon_struct.szInfo, TEXT("text"), szInfo_LENGTH);
		*/

		icon_struct.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		icon_struct.hBalloonIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_DEFAULTCOLOR);
	}
	
	/**
	* производит инициализацию контекстного меню context_menu
	* @see App::context_menu
	*/
	void init_context_menu(){
		context_menu = CreatePopupMenu();
		AppendMenu(context_menu, MFT_STRING | MF_CHECKED, CM_EXEC, TEXT("Действие"));
		AppendMenu(context_menu, MFT_STRING, CM_EXIT, TEXT("&Выход"));
	}

	/**
	* открывает файл по стандартному пути (%USERPROFILE%\Documents\Happy Birthday\birthdays.csv)
	* или по переданному пути
	* @param user_path - указатель на строку путь к файлу
	* @return NULL или то что возвращает CreateFile
	* INVALID_HANDLE_VALUE
	*/
	HANDLE open_birthdays_file(LPCTSTR user_path = NULL){

		if(user_path){
			return CreateFile(user_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}


		HKEY hKey = nullptr;
		HANDLE h_process_heap = GetProcessHeap();

		//получим путь к папке мои документы для текущего юзера из реестра
		if(RegOpenKeyEx(
			HKEY_CURRENT_USER,
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"),
			0,
			KEY_QUERY_VALUE,
			&hKey
		) != ERROR_SUCCESS){
			return NULL;
		}

		//получим размер строки в реестре
		DWORD type, data_size;
		if(RegQueryValueEx(hKey, TEXT("Personal"), NULL, &type, NULL, &data_size) != ERROR_SUCCESS){
			RegCloseKey(hKey);
			return NULL;
		}

		if(type != REG_EXPAND_SZ && type != REG_SZ || data_size == 0){
			RegCloseKey(hKey);
			return NULL;
		}

		LPTSTR path = (LPTSTR)HeapAlloc(h_process_heap, 0, data_size + sizeof(TEXT("\\Happy Birthday\\birthdays.csv")));
		if(path == NULL){
			RegCloseKey(hKey);
			return NULL;
		}

		//вытащим строку из реестра
		if(RegQueryValueEx(hKey, TEXT("Personal"), NULL, &type, (LPBYTE)path, &data_size) != ERROR_SUCCESS){
			HeapFree(h_process_heap, 0, path);
			RegCloseKey(hKey);
			return NULL;
		}
		
		RegCloseKey(hKey);

		//добавим свой путь
		if(lstrcat(path, TEXT("\\Happy Birthday\\birthdays.csv")) == NULL){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		//заменим системные переменные
		data_size = ExpandEnvironmentStrings(path, NULL, 0);
		if(data_size == 0){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		LPTSTR full_path = (LPTSTR)HeapAlloc(h_process_heap, 0, data_size * sizeof(TCHAR));
		if(full_path == NULL){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		data_size = ExpandEnvironmentStrings(path, full_path, data_size);
		if(data_size == 0){
			HeapFree(h_process_heap, 0, path);
			HeapFree(h_process_heap, 0, full_path);
			return NULL;
		}

		//откроем файл на чтение
		HANDLE ret = CreateFile(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		HeapFree(h_process_heap, 0, path);
		HeapFree(h_process_heap, 0, full_path);

		return ret;
	}
};

App* application = nullptr;


/**
* обрабатывает события приложения
* @see App::create_window
* @param window - 
* @param message - 
* WM_SHELLNOTIFY - Сообщение от значка (App::show_context_menu)
* WM_COMMAND - событие выбора пункта меню (App::context_menu_click)
* WM_DESTROY - скрывает значок(App::hide_icon), выход из программы
*/
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		//Сообщение от значка
		case WM_SHELLNOTIFY:
			if(LOWORD(lParam) == WM_CONTEXTMENU){
				KillTimer(window, CLOSE_TIMER);
				application->show_context_menu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			}
			break;

		//событие выбора пункта меню
		case WM_COMMAND:
			application->context_menu_click(LOWORD(wParam));
			break;
		
		//события таймеров
		case WM_TIMER:
			switch(wParam){
				case CLOSE_TIMER:
					KillTimer(window, CLOSE_TIMER);
					DestroyWindow(window);
					break;
 
				case BALLON_TYPES_INTERVAL_TIMER:
					application->show_birthdays_ballons();
					break; 
			}
			break;
			
		//выход из программы
		case WM_DESTROY:
			application->hide_icon();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}



/**
* точка входа
*/
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance, lpCmdLine);

	application = &program;

	return program.start();
}