#include <iostream>
#include <vector>
#include <climits>   //for CHAR_BIT
#include <string>
#include <type_traits>
#include <numeric>
#include <assert.h>
#include <thread>
#include <functional>

using namespace std;

//RefIn == False
//RefOut == False
//XorOut == 0
namespace crc
{
	namespace algorithm
	{
		template<class T>
		struct parameters;

		template<>
		struct parameters<uint32_t> {
			uint32_t shift;
			uint32_t only_first_byte_is_1_mask;
			uint32_t polynom;
			uint32_t init;

			constexpr parameters(uint32_t polynom, uint32_t init) :
				shift(24), polynom(polynom), init(init), only_first_byte_is_1_mask(0x80000000) {}
		};

		template<>
		struct parameters<uint16_t> {
			uint16_t shift;
			uint16_t only_first_byte_is_1_mask;
			uint16_t polynom;
			uint16_t init;

			constexpr parameters(uint16_t polynom, uint16_t init) :
				shift(8), polynom(polynom), init(init), only_first_byte_is_1_mask(0x8000) {}
		};

		template<>
		struct parameters<uint8_t> {
			uint8_t shift;
			uint8_t only_first_byte_is_1_mask;
			uint8_t polynom;
			uint8_t init;

			constexpr parameters(uint8_t polynom, uint8_t init) :
				shift(0), polynom(polynom), init(init), only_first_byte_is_1_mask(0x80) {}
		};

		namespace bit 
		{
			namespace _32
			{
				constexpr parameters<uint32_t> _MPEG_2(0x04C11DB7, 0xFFFFFFFF);
				constexpr parameters<uint32_t> _Q(0x814141AB, 0);
				constexpr parameters<uint32_t> _XFER(0x000000AF, 0);
			}

			namespace _16
			{
				constexpr parameters<uint16_t> _CCITT_FALSE(0x1021, 0xFFFF);
				constexpr parameters<uint16_t> _AUG_CCITT(0x1021, 0x1D0F);
				constexpr parameters<uint16_t> _BUYPASS(0x8005, 0);
				constexpr parameters<uint16_t> _CDMA2000(0xC867, 0xFFFF);
				constexpr parameters<uint16_t> _DDS_110(0x8005, 0x800D);
				constexpr parameters<uint16_t> _DECT_X(0x0589, 0);
				constexpr parameters<uint16_t> _T10_DIF(0x8BB7, 0);
				constexpr parameters<uint16_t> _TELEDISK(0xA097, 0);
				constexpr parameters<uint16_t> _XMODEM(0x1021, 0);
			}

			namespace _8
			{
				constexpr parameters<uint8_t> _CDMA2000(0x9B, 0xFF);
				constexpr parameters<uint8_t> _DVB_S2(0xD5, 0);
				constexpr parameters<uint8_t> _I_CODE(0x1D, 0xFD);
				constexpr parameters<uint8_t> _(0x07, 0);
			};
		}
		
	}

	template <class T>
	T calculate(const char* p_data, const size_t len, const algorithm::parameters<T> params)
	{
		T crc = params.init;
		size_t idx = len;
		while (idx--) {
			crc ^= *p_data++ << params.shift;

			for (unsigned i = 0; i < CHAR_BIT; i++)
				crc = crc & params.only_first_byte_is_1_mask ? (crc << 1) ^ params.polynom : crc << 1;
		}
		return crc;
	}
};

vector<uint8_t> get_uint8_representation(const uint8_t v) {
	vector<uint8_t> ret(1);
	ret[0] = v ;
	return ret;
}

vector<uint8_t> get_uint8_representation(const uint16_t v) {
	vector<uint8_t> ret(2);
	ret[0] = v >> 8;
	ret[1] = v;
	return ret;
}


vector<uint8_t> get_uint8_representation(const uint32_t v) {
	vector<uint8_t> ret(4);
	ret[0] = v >> 24;
	ret[1] = v >> 16;
	ret[2] = v >> 8;
	ret[3] = v;
	return ret;
}

template<class T>
bool has_data_been_transferred_correctly(const char* p_data,
	const size_t len,
	const crc::algorithm::parameters<T> params) {

	size_t num_of_crc_bytes;
	if constexpr (is_same_v<T, uint32_t>) {
		num_of_crc_bytes = 4;
		
	}
	else if constexpr (is_same_v<T, uint16_t>) {
		num_of_crc_bytes = 2;
	}
	else if constexpr (is_same_v<T, uint8_t>) {
	num_of_crc_bytes = 1;
	}
	vector<char> data;
	const size_t actual_data_size = len - num_of_crc_bytes;
	data.reserve(actual_data_size);
	move(p_data, p_data + actual_data_size, back_inserter(data));

	T crc_expected = crc::calculate(p_data, len-num_of_crc_bytes, params);
	vector<uint8_t> crc_expected_8_bit_representation(len);
	crc_expected_8_bit_representation = get_uint8_representation(crc_expected);

	vector<uint8_t> crc_actual;
	crc_actual.reserve(num_of_crc_bytes);
	move(p_data + actual_data_size , p_data + len , back_inserter(crc_actual));

	return crc_expected_8_bit_representation == crc_actual;
}

//@see https://crccalc.com/
void crc_tests() {
	const string check = "123456789";
	const size_t len = check.length();
	const char* const p_ckeck = check.data();
	//32 bit
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_32::_MPEG_2) == 0x0376E6E7);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_32::_Q) == 0x3010BF7F);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_32::_XFER) == 0xBD0BE338);

	//16 bit
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_CCITT_FALSE) == 0x29B1);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_AUG_CCITT) == 0xE5CC);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_BUYPASS) == 0xFEE8);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_CDMA2000) == 0x4C06);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_DDS_110) == 0x9ECF);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_DECT_X) == 0x007F);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_T10_DIF) == 0xD0DB);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_TELEDISK) == 0x0FB3);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_16::_XMODEM) == 0x31C3);

	//8 bit
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_8::_) == 0xF4);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_8::_CDMA2000) == 0xDA);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_8::_DVB_S2) == 0xBC);
	assert(crc::calculate(p_ckeck, len, crc::algorithm::bit::_8::_I_CODE) == 0x7E);

	//Ù stands for 244 ascii == 0xF4 == check for crc8 algorithm
	const string data_crc8_ok = "123456789Ù";
	assert(has_data_been_transferred_correctly(data_crc8_ok.data(), data_crc8_ok.length(), crc::algorithm::bit::_8::_));

	const string data_crc8_not_ok = "123056789Ù";
	assert(has_data_been_transferred_correctly(data_crc8_not_ok.data(), data_crc8_not_ok.length(), crc::algorithm::bit::_8::_) == false);

	const string data_crc8_not_ok_crc = "123456789a";
	assert(has_data_been_transferred_correctly(data_crc8_not_ok_crc.data(), data_crc8_not_ok_crc.length(), crc::algorithm::bit::_8::_) == false);

	//'1' + '√' stands for 0x31C3 == check for crc16_XMODEM algorithm
	const string data_crc16_ok = "1234567891√"; 
	assert(has_data_been_transferred_correctly(data_crc16_ok.data(), data_crc16_ok.length(), crc::algorithm::bit::_16::_XMODEM));

	const string data_crc16_not_ok = "1234567Ù‚˚Ù˚‚Ù‚˚891√";
	assert(has_data_been_transferred_correctly(data_crc16_not_ok.data(), data_crc16_not_ok.length(), crc::algorithm::bit::_16::_XMODEM) == false);

	const string data_crc16_not_ok_crc = "1234567892√";
	assert(has_data_been_transferred_correctly(data_crc16_not_ok_crc.data(), data_crc16_not_ok_crc.length(), crc::algorithm::bit::_16::_XMODEM) == false);
}

int main() {
	
	thread th(crc_tests);
	if (th.joinable()) {
		th.join();
	}else{
		th.detach(); 
	}
	
	system("pause");
	return 0;
}