#include "keilo_auth.hpp"

#include <rsa.h>
#include <osrng.h>
#include <base64.h>
#include <files.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "cryptlib")

#pragma warning(disable:4996)

using namespace CryptoPP;

keilo_auth::keilo_auth() : socket_(0), port_(0), is_path_set_(false), is_file_set_(false)
{
}

keilo_auth::keilo_auth(const int port, const std::string path, const std::initializer_list<std::string>&& key_files) :
	socket_(0), port_(port),
	path_(path)
{
	is_path_set_ = !path_.empty();
	if (is_file_set_ = key_files.size() == 4; is_file_set_)
	{
		auto pos = 0;
		for (auto it = key_files.begin(); it != key_files.end(); ++it)
			key_files_[pos++] = *it;
	}
}


keilo_auth::~keilo_auth()
{
	clean_up();
}

void keilo_auth::run()
{
	if (!initialize_socket())
		throw std::exception("failed to initialize winsock");
	if (!is_path_set_)
		throw std::exception("file path do not set.");
	if (!is_file_set_)
		throw std::exception("key files does not set.");
	
	if (listen(socket_, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception("failed to listening");

	printf("Successfully started auth server.\n");

	while (true)
	{
		sockaddr_in clnt_addr;
		int clnt_addr_size = sizeof clnt_addr;

		const auto clnt = accept(
			socket_,
			reinterpret_cast<sockaddr*>(&clnt_addr),
			&clnt_addr_size
		);
		if (clnt == INVALID_SOCKET)
			continue;

		printf("[%s:%d] connected.\n", inet_ntoa(clnt_addr.sin_addr), clnt_addr.sin_port);

		workers_.emplace_back([this, client = clnt]()
		{
			while (true)
			{
				auto buffer = read(client);
				const auto flag = get_flag(buffer);
				switch (flag)
				{
				case 0: // s(erver)en(crypt)
				case 1: // c(lient)en(crypt)
				{
					// auth

					if (buffer = read(client); auth_keys_[flag] != buffer)
						write(client, "fail");
					else
					{
						write(client, "success");
						write(client, encode_with_base64(
							encrypt_with_rsa(
								flag, read(client))
						)
						);
					}
				}
				break;

				case 2: // s(erver)de(crypt)
				case 3: // c(lient)de(crypt)
				{
					write(
						client, decrypt_with_rsa(
							flag, decode_with_base64(read(client))
						)
					);
				}
				break;

				default:
					break;
				}
			}
			closesocket(client);
		});
	}
}

bool keilo_auth::initialize_socket()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0)
		return false;

	if (socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); socket_ == INVALID_SOCKET)
		return false;

	ZeroMemory(&address_, sizeof sockaddr_in);

	address_.sin_family = AF_INET;
	address_.sin_addr.S_un.S_addr = INADDR_ANY;
	address_.sin_port = htons(6062);

	if (bind(socket_, reinterpret_cast<sockaddr*>(&address_), sizeof address_) == SOCKET_ERROR)
		return false;
	
	return true;
}

void keilo_auth::clean_up()
{
	closesocket(socket_);

	for (auto& worker : workers_)
		if (worker.joinable())
			worker.join();
}

void keilo_auth::write(const SOCKET socket, std::string data)
{
	if (send(socket, data.c_str(), data.length(), 0) == SOCKET_ERROR)
	{
		closesocket(socket);
		throw std::exception("send error");
	}
}

std::string keilo_auth::read(const SOCKET socket)
{
	char buffer[512];
	const auto received = recv(socket, buffer, 512, 0);

	if (received <= 0)
	{
		closesocket(socket);
		throw std::exception("recv error");
	}
	buffer[received] = 0;

	return buffer;
}

void keilo_auth::issue_rsa_key()
{
	// issue private key
	AutoSeededRandomPool rng;
	InvertibleRSAFunction private_key;
	private_key.Initialize(rng, 4096);

	Base64Encoder encoded_private_key(new FileSink(R"(private_key.txt)"));
	private_key.DEREncode(encoded_private_key);
	encoded_private_key.MessageEnd();

	// issue public key from private key
	const auto public_key((static_cast<RSAFunction>(private_key)));

	Base64Encoder encoded_public_key(new FileSink(R"(public_key.txt)"));
	public_key.DEREncode(encoded_public_key);
	encoded_public_key.MessageEnd();
}

std::string keilo_auth::encode_with_base64(const std::string data)
{
	std::string encoded;
	StringSource(
		data, true,
		new Base64Encoder(new StringSink(encoded))
	);

	return encoded;
}

std::string keilo_auth::decode_with_base64(const std::string data)
{
	std::string decoded;
	StringSource(
		data, true,
		new Base64Decoder(new StringSink(decoded))
	);

	return decoded;
}

std::string keilo_auth::encrypt_with_rsa(const int key, const std::string data) const
{
	FileSource b64_encrypted_key(
		(path_ + key_files_[key]).c_str(),
		true, new Base64Decoder
	);
	const RSAES_OAEP_SHA_Encryptor public_key(b64_encrypted_key);
	AutoSeededRandomPool auto_seeded_random_pool;

	std::string rsa_encrypted_data;

	StringSource(
		data, true,
		new PK_EncryptorFilter(
			auto_seeded_random_pool, public_key,
			new StringSink(rsa_encrypted_data)
		)
	);

	return rsa_encrypted_data;
}

std::string keilo_auth::decrypt_with_rsa(const int key, const std::string data) const
{
	FileSource b64_encrypted_key(
		(path_ + key_files_[key]).c_str(),
		true, new Base64Decoder
	);
	const RSAES_OAEP_SHA_Decryptor private_key(b64_encrypted_key);
	AutoSeededRandomPool auto_seeded_random_pool;

	std::string rsa_decrypted_data;

	StringSource(data, true,
		new PK_DecryptorFilter(
			auto_seeded_random_pool, private_key,
			new StringSink(rsa_decrypted_data)
		)
	);

	return rsa_decrypted_data;
}

int keilo_auth::get_flag(const std::string flag)
{
	if (flag == "sen")
		return 0;
	if (flag == "cen")
		return 1;
	if (flag == "sde")
		return 2;
	if (flag == "cde")
		return 3;
	return -1;
}
