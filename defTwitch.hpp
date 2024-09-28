#ifndef DEF_TWITCH_HPP
#define DEF_TWITCH_HPP

#include <string>
#include <thread>
#include <atomic>
#include <algorithm>

#ifdef _WIN32
	#include <WinSock2.h>
	#include <WS2tcpip.h>

	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <netdb.h>
#endif

#ifdef _WIN32
	typedef SOCKET Socket;
	#define InvalidSocket INVALID_SOCKET
	#define SocketError SOCKET_ERROR
#else
	typedef int Socket;
	#define InvalidSocket -1
	#define SocketError -1
#endif

namespace def::twitch
{
	constexpr size_t RESPONCE_SIZE = 1024;
	constexpr uint32_t INVALID_AUTHOR_ID = uint32_t(-1);

	struct Message
	{
		enum class Type
		{
			MESSAGE,
			JOIN,
			LEAVE,
			INVALID
		} type = Type::INVALID;

		struct
		{
			uint32_t id;
			std::string name;

			bool isMod;
		} author;

		std::string id;
		std::string text;
	};

	class Chat
	{
	public:
		Chat();
		Chat(const std::string& auth, const std::string& nick);
		virtual ~Chat();

	public:
		bool Initialise(const std::string& oauth, const std::string& nick);
		void Start();

		void Join(const std::string& channel);
		void Leave();

		void Send(const std::string& text);
		void Reply(const std::string& message_id, const std::string& text);

		virtual bool OnMessage(const Message& message) = 0;

	private:
		bool ConnectToTwitch(const uint32_t port);
		void SendToSocket(const std::string& text);

		void MainLoop();

	private:
		Socket m_Socket;

		std::thread m_Thread;
		std::atomic<bool> m_Running;

		std::string m_Channel;

	};
}

#endif
