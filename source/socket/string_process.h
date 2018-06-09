#pragma once

#include <string>

#include <CryptoPP/cryptlib.h>
#include <CryptoPP/base64.h>
#include <CryptoPP/filters.h>
#include <CryptoPP/des.h>
#include <CryptoPP/modes.h>

#include <zlib-1.2.11/zlib.h>

#pragma comment(lib, "cryptlib")
#pragma comment(lib, "zlibstat")

class string_process {
public:
	static std::string compress(const std::string& str) {
		z_stream stream;
		memset(&stream, 0, sizeof stream);

		deflateInit(&stream, Z_BEST_COMPRESSION);

		stream.next_in = (Bytef*)str.data();
		stream.avail_in = str.size();

		int ret;
		char buffer[32768];
		std::string out;

		do {
			stream.next_out = reinterpret_cast<Bytef*>(buffer);
			stream.avail_out = sizeof buffer;

			ret = deflate(&stream, Z_FINISH);
			if (out.size() < stream.total_out)
				out.append(buffer, stream.total_out - out.size());
		} while (ret == Z_OK);

		deflateEnd(&stream);

		return ecb_encrypt<CryptoPP::DES>(key_, out);
	}

	static std::string decompress(const std::string& str) {
		auto data = ecb_decrypt<CryptoPP::DES>(key_, str);

		z_stream stream;
		memset(&stream, 0, sizeof stream);

		inflateInit(&stream);

		stream.next_in = (Bytef*)data.data();
		stream.avail_in = data.size();

		int ret;
		char buffer[32768];
		std::string out;

		do {
			stream.next_out = reinterpret_cast<Bytef*>(buffer);
			stream.avail_out = sizeof buffer;

			ret = inflate(&stream, 0);

			if (out.size() < stream.total_out)
				out.append(buffer, stream.total_out - out.size());
		} while (ret == Z_OK);

		inflateEnd(&stream);

		return out;
	}

private:
	template <class Ty>
	static std::string encrypt(Ty& encryptor, const std::string& plain) {
		std::string encoded;

		try {
			CryptoPP::StringSource(
				plain, true,
				new CryptoPP::StreamTransformationFilter(
					encryptor, new CryptoPP::Base64Encoder(
						new CryptoPP::StringSink(encoded), false),
					CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING));
		}
		catch (...) {
		}

		return encoded;
	}
	template <class Ty>
	static std::string decrypt(Ty& decryptor, const std::string& encoded) {
		std::string recovered;

		try {
			CryptoPP::StringSource(
				encoded, true,
				new CryptoPP::Base64Decoder(new CryptoPP::StreamTransformationFilter(
					decryptor, new CryptoPP::StringSink(recovered),
					CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING)));
		}
		catch (...) {
		}

		return recovered;
	}
	template <class Ty>
	static std::string ecb_encrypt(byte* const key, const std::string& plain) {
		typename CryptoPP::ECB_Mode<Ty>::Encryption encryptor(
			key, Ty::DEFAULT_KEYLENGTH);
		return encrypt(encryptor, plain);
	}
	template <class Ty>
	static std::string ecb_decrypt(byte* const key, const std::string& plain) {
		typename CryptoPP::ECB_Mode<Ty>::Decryption decryptor(
			key, Ty::DEFAULT_KEYLENGTH);
		return decrypt(decryptor, plain);
	}

	static byte key_[CryptoPP::DES::DEFAULT_KEYLENGTH];
};
