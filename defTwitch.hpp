#ifndef DEF_TWITCH_HPP
#define DEF_TWITCH_HPP

#include <string>
#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

namespace def::twitch
{
	constexpr size_t RESPONCE_SIZE = 128;

	class Chat
	{
	public:
		Chat() = default;
		Chat(const std::string& auth, const std::string& nick);
		virtual ~Chat();

	public:
		bool Initialise(const std::string& oauth, const std::string& nick);
		void Start();

		void Join(const std::string& channel);
		void Leave();

		void Send(const std::string& text);
		void Reply(const std::string& message_id, const std::string& text);

		virtual bool OnUserMessage(const std::string& message_id, const std::string& name, const std::string& text) = 0;

	private:
		static bool InitialiseWSA();

		bool ConnectToTwitch(const uint32_t port);
		void SendToSocket(const std::string& text);

		void MainLoop();

	private:
		SOCKET m_Socket;

		std::thread m_Thread;
		std::atomic<bool> m_Running;

		std::string m_Channel;

	};
}

#endif
