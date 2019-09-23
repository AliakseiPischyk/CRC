#include <iostream>
#include <vector>
#include <climits>   //for CHAR_BIT
#include <string>
#include <type_traits>
#include <numeric>

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

bool func(const char* p_data, const size_t len, const size_t crc_bytes) {
	return true;
}

int main() {
	auto a = get_uint8_representation(crc::calculate("123456789", 9, crc::algorithm::bit::_16::_AUG_CCITT));
	vector<uint8_t> test;
	const string data = "123456789";
	for (const auto c : data) {
		test.push_back(c);
	}
	auto crc_uint8_format = get_uint8_representation(crc::calculate(data.data(), data.length(), crc::algorithm::bit::_16::_AUG_CCITT));
	for (const auto v : crc_uint8_format) {
		test.push_back(v);
	}
	for (auto v : crc_uint8_format) {
		cout <<hex<<int(v);
	}
	
	system("pause");
	return 0;
}