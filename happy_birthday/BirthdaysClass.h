#pragma once

#include "BirthdayList.h"

class BirthdaysClass{
	private:
		BirthdayList birthday_list[12];
	public:
		BirthdaysClass();

		/**
		* ƒобавл€ет день рождени€, распредел€ет их по мес€цам
		* 
		*/
		bool add_birthday(SYSTEMTIME date, const String& label);

		/**
		* @param month_number - номер мес€ца, количество дней рождений в котором будет возвращено; 0 - возвращает количество всех дней рождений
		*/
		UINT32 get_count_birthdays(UINT8 month_number=0);

		/**
		* производит разбор файла, извлекает построчно данные, строка имеет формат
		* бирка\tдата\n
		* @param input - дескриптор открытого файла с правами на чтение
		* @returns количество добавленных дней рождений
		*/
		int parse_data_from_file(HANDLE input);

		BirthdayList get_birthdays(UINT8 month, UINT8 day=0);

		~BirthdaysClass();
	private:
		/**
		* @param str - строка в которой ищутс€ символы конца
		* @param end_char_pos - сюда поместитс€ индекс конечного символа (если такой не будет найден будет иметь значение размера строки)
		* @returns \0, \r, \n, DELIMITER_CHAR
		* @see DELIMITER_CHAR
		*/
		char get_end_char_pos(std::string& str, DWORD& end_char_pos);

		/**
		* @param str - строка в которой ищутс€ символы конца
		* @param start_pos - позици€ символа с которого начинаетс€ поиск неконечного символа
		* @returns позици€ первого неконечного символа или длину строки если смвол не найден
		*/
		DWORD get_not_end_char_pos(std::string& str, DWORD start_pos);

		/**
		* @param str - строка с датой формата yyyy.mm.dd или dd.mm.yyyy, точка - любой нецифровой символ
		* @param date - структура котора€ будет заполнена датой, если парсинг будет удачным
		* @returns успешность парсинга
		*/
		bool str_to_systemtime(std::string str, SYSTEMTIME& date);
};