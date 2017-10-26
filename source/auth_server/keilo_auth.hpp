#pragma once
#include <string>
#include <list>
#include <winsock2.h>
#include <thread>


class keilo_auth
{
public:
	keilo_auth();
	keilo_auth(const int port, const std::string path, const std::initializer_list<std::string>&& key_files);
	~keilo_auth();

	void run();

#pragma region WinSock
private:
	bool initialize_socket();
	void clean_up();
	static void write(SOCKET socket, std::string data);
	static std::string read(SOCKET socket);

	WSAData wsa_;
	SOCKET socket_;
	SOCKADDR_IN address_;
	int port_;
	std::list<std::thread> workers_;
#pragma endregion
#pragma region Crypto
	static void issue_rsa_key();
	static std::string encode_with_base64(std::string data);
	static std::string decode_with_base64(std::string data);
	std::string encrypt_with_rsa(int key, std::string data) const;
	std::string decrypt_with_rsa(int key, std::string data) const;
	static int get_flag(std::string flag);

	std::string path_;
	bool is_path_set_;
	std::string key_files_[4];
	bool is_file_set_;

	const std::string auth_keys_[2] = {
		"", // server
		""  // client
	};
#pragma endregion
};

