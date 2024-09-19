#include "defTwitch.hpp"

namespace def::twitch
{
	Chat::Chat(const std::string& auth, const std::string& nick)
	{
		Initialise(auth, nick);
	}

	Chat::~Chat()
	{
		WSACleanup();
	}

	bool Chat::Initialise(const std::string& oauth, const std::string& nick)
	{
		if (!InitialiseWSA())
			return false;

		if (!ConnectToTwitch(6667))
			return false;

		SendToSocket("PASS oauth:" + oauth + "\r\n");
		SendToSocket("NICK " + nick + "\r\n");
		SendToSocket("CAP REQ :twitch.tv/tags twitch.tv/commands twitch.tv/membership\r\n");

		char responce[RESPONCE_SIZE];
		std::string buffer;

		while (1)
		{
			int count = recv(m_Socket, responce, 128, 0);

			for (int i = 0; i < count; i++)
			{
				buffer.append(1, responce[i]);

				if (responce[i] == '\n')
				{
					if (buffer.find(":tmi.twitch.tv NOTICE *") == 0)
					{
						if (buffer.find(":Login authentication failed") ||
							buffer.find(":Improperly formatted auth"))
							return false;
					}
					
					return true;
				}
			}
		}

		return false;
	}

	void Chat::Start()
	{
		m_Running = true;
		m_Thread = std::thread(&Chat::MainLoop, this);

		if (m_Thread.joinable())
			m_Thread.join();
	}

	void Chat::Join(const std::string& channel)
	{
		SendToSocket("JOIN #" + channel + "\r\n");
		m_Channel = channel;
	}

	void Chat::Leave()
	{
		SendToSocket("PART #" + m_Channel + "\r\n");
		m_Channel.clear();
	}

	void Chat::Send(const std::string& text)
	{
		SendToSocket("PRIVMSG #" + m_Channel + " :" + text + "\r\n");
	}

	void Chat::Reply(const std::string& message_id, const std::string& text)
	{
		SendToSocket("@reply-parent-msg-id=" + message_id + " PRIVMSG #" + m_Channel + " :" + text + "\r\n");
	}

	bool Chat::InitialiseWSA()
	{
		WSADATA wsaData;
		return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
	}

	bool Chat::ConnectToTwitch(const uint32_t port)
	{
		struct addrinfo* addr = nullptr;

		if (getaddrinfo("irc.chat.twitch.tv", std::to_string(port).c_str(), nullptr, &addr) != 0)
			return false;

		m_Socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

		if (m_Socket == INVALID_SOCKET)
			return false;

		if (connect(m_Socket, addr->ai_addr, addr->ai_addrlen) == SOCKET_ERROR)
			return false;

		return true;
	}

	void Chat::SendToSocket(const std::string& text)
	{
		send(m_Socket, text.c_str(), text.length(), 0);
	}

	void Chat::MainLoop()
	{
		char responce[RESPONCE_SIZE];
		std::string buffer;

		while (m_Running)
		{
			int count = recv(m_Socket, responce, 128, 0);

			for (int i = 0; i < count; i++)
			{
				buffer.append(1, responce[i]);

				if (responce[i] == '\n')
				{
					if (buffer.find("PRIVMSG") != std::string::npos)
					{
						size_t message_info_start = buffer.find("user-type=");
						size_t colon = buffer.find(':', message_info_start) + 1;
						size_t after_name = buffer.find('!', message_info_start);
						size_t before_message = buffer.find(':', after_name);
						size_t message_id_start = buffer.find("id=") + 3;
						size_t message_id_end = buffer.find(';', message_id_start);

						std::string name = buffer.substr(colon, after_name - colon);
						std::string message = buffer.substr(before_message + 1);
						std::string message_id = buffer.substr(message_id_start, message_id_end - message_id_start);

						// Remove \r\n
						message.pop_back();
						message.pop_back();

						if (!OnUserMessage(message_id, name, message))
							m_Running = false;
					}

					buffer.clear();
				}
			}
		}
	}
}
