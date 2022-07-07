#pragma once
class BirthdayList{
	private:
		std::vector<String> labels;
		std::vector<SYSTEMTIME> dates;

	public:
		BirthdayList();

		UINT32 size() const;

		/**
		* Добавляет день рождения,
		* 
		*/
		bool add_birthday(SYSTEMTIME date, const String &label);
		BirthdayList get_birthdays(UINT8 day=0) const;
		const SYSTEMTIME& get_birthday_date(UINT32 ind) const;

		~BirthdayList();
};