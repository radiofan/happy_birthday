#pragma once

#include "BirthdayList.h"

class BirthdaysClass{
	private:
		BirthdayList birthday_list[12];
	public:
		BirthdaysClass();

		/**
		* ��������� ���� ��������, ������������ �� �� �������
		* 
		*/
		bool add_birthday(SYSTEMTIME date, const String& label);

		/**
		* @param month_number - ����� ������, ���������� ���� �������� � ������� ����� ����������; 0 - ���������� ���������� ���� ���� ��������
		*/
		UINT32 get_count_birthdays(UINT8 month_number=0);

		/**
		* ���������� ������ �����, ��������� ��������� ������, ������ ����� ������
		* �����\t����\n
		* @param input - ���������� ��������� ����� � ������� �� ������
		* @returns ���������� ����������� ���� ��������
		*/
		int parse_data_from_file(HANDLE input);

		BirthdayList get_birthdays(UINT8 month, UINT8 day=0);

		~BirthdaysClass();
	private:
		/**
		* @param str - ������ � ������� ������ ������� �����
		* @param end_char_pos - ���� ���������� ������ ��������� ������� (���� ����� �� ����� ������ ����� ����� �������� ������� ������)
		* @returns \0, \r, \n, DELIMITER_CHAR
		* @see DELIMITER_CHAR
		*/
		char get_end_char_pos(std::string& str, DWORD& end_char_pos);

		/**
		* @param str - ������ � ������� ������ ������� �����
		* @param start_pos - ������� ������� � �������� ���������� ����� ����������� �������
		* @returns ������� ������� ����������� ������� ��� ����� ������ ���� ����� �� ������
		*/
		DWORD get_not_end_char_pos(std::string& str, DWORD start_pos);

		/**
		* @param str - ������ � ����� ������� yyyy.mm.dd ��� dd.mm.yyyy, ����� - ����� ���������� ������
		* @param date - ��������� ������� ����� ��������� �����, ���� ������� ����� �������
		* @returns ���������� ��������
		*/
		bool str_to_systemtime(std::string str, SYSTEMTIME& date);
};