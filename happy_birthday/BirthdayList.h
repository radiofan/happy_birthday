#pragma once
class BirthdayList{
	private:
		std::vector<String> labels;
		std::vector<SYSTEMTIME> dates;

	public:
		BirthdayList();
		/**
		* склонение слов после числа
		* 
		* @param number - число
		* @param title_1 - вариант для склонения для 1
		* @param title_2 - вариант для склонения для 2
		* @param title_10 - вариант для склонения для 10
		* 
		* @returns строки типа "1 книга", "2 книги", "10 книг"
		*/
		static String num_decline(INT32 number, const String& title_1, const String& title_2, const String& title_10);

		UINT32 size() const;

		/**
		* Добавляет день рождения,
		* 
		*/
		bool add_birthday(SYSTEMTIME date, const String& label);
		BirthdayList get_birthdays(UINT8 day=0) const;
		const SYSTEMTIME& get_date(UINT32 ind) const;
		const String& get_label(UINT32 ind) const;
		String get_greeting_with_day_age(UINT32 ind, const SYSTEMTIME& current_time) const;

		~BirthdayList();
};