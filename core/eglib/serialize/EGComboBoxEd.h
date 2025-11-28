// (c) 2018 Beem Media

#pragma once

typedef std::function<void(EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )> egComboBoxPopulateFn;

struct eg_combo_box_str_ed
{
	eg_d_string          String;
	egComboBoxPopulateFn PopulateCb = nullptr;

	eg_combo_box_str_ed() = default;
	eg_combo_box_str_ed( eg_cpstr InStr ): String( InStr ) { }
};

struct eg_combo_box_crc_ed
{
	eg_string_crc        Crc;
	egComboBoxPopulateFn PopulateCb = nullptr;

	eg_combo_box_crc_ed() = default;
	eg_combo_box_crc_ed( eg_string_crc InCrc ): Crc( InCrc ) { }

	void operator = ( const eg_string_crc& rhs ) { Crc = rhs; }
	operator eg_string_crc () const { return Crc; }
};
