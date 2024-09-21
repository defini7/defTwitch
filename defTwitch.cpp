#include "defTwitch.hpp"

namespace def::twitch
{
	Chat::Chat()
	{
		m_Socket = INVALID_SOCKET;
	}

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

		u_long mode = 0;

		if (ioctlsocket(m_Socket, FIONBIO, &mode) != NO_ERROR)
			return false;

		char responce[RESPONCE_SIZE];
		std::string buffer;

		while (1)
		{
			int count = recv(m_Socket, responce, RESPONCE_SIZE, 0);

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
			int count = recv(m_Socket, responce, RESPONCE_SIZE, 0);

			for (int i = 0; i < count; i++)
			{
				buffer.append(1, responce[i]);

				if (responce[i] == '\n')
				{
					if (buffer == "PING :tmi.twitch.tv\r\n")
						SendToSocket("PONG :tmi.twitch.tv\r\n");
					else
					{
						auto has = [&buffer = buffer](const std::string& text, size_t& pos)
							{
								pos = buffer.find(text);
								return pos != std::string::npos;
							};

						size_t pos = buffer.find("tmi.twitch.tv");

						std::string author;

						if (pos != std::string::npos)
						{
							pos -= 2;

							do
							{
								author.push_back(buffer[pos--]);
							} while (buffer[pos] != '@' && buffer[pos] != ':');

							std::reverse(author.begin(), author.end());
						}

						if (has("PRIVMSG", pos))
						{
							Message msg;

							auto get_tag = [&buffer = buffer](const std::string& tag)
								{
									size_t start = buffer.find(tag) + tag.length() + 1;
									size_t end = buffer.find(';', start);
									return buffer.substr(start, end - start);
								};

							size_t message_start = buffer.find(':', pos) + 1;
							msg.text = buffer.substr(message_start, buffer.size() - message_start - 2);

							msg.type = Message::Type::MESSAGE;
							msg.id = get_tag("id");

							msg.author.name = author;
							msg.author.id = std::stoul(get_tag("user-id"));
							msg.author.isMod = get_tag("mod") == "1";

							if (!OnMessage(msg))
								m_Running = false;
						}
						else if (has("JOIN", pos))
						{
							Message msg;

							msg.author.name = author;
							msg.author.id = INVALID_AUTHOR_ID;
							msg.type = Message::Type::JOIN;

							if (!OnMessage(msg))
								m_Running = false;
						}
						else if (has("PART", pos))
						{
							Message msg;

							msg.author.name = author;
							msg.author.id = INVALID_AUTHOR_ID;
							msg.type = Message::Type::LEAVE;

							if (!OnMessage(msg))
								m_Running = false;
						}
					}

					buffer.clear();
				}
			}
		}
	}
}
