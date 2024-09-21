#include <iostream>
#include <fstream>
#include <string>

#include "defTwitch.hpp"

bool ReadFile(const char* filename, std::string& output)
{
	std::ifstream ifs(filename);
	if (!ifs.is_open()) return false;

	while (!ifs.eof())
	{
		std::string buffer;
		std::getline(ifs, buffer);

		output += buffer + '\n';
	}

	return true;
}

class MyChat : public def::twitch::Chat
{
public:
	MyChat()
	{
		std::string oauth;

		if (ReadFile("auth.txt", oauth))
		{
			oauth.pop_back();

			Initialise(oauth, "def1ni7");

		}
		else
			std::cerr << "can't read auth.txt file" << std::endl;

		srand(time(0));
	}

protected:
	bool OnMessage(const def::twitch::Message& message) override
	{
		switch (message.type)
		{
		case def::twitch::Message::Type::JOIN: std::cout << "User " << message.author.name << " has joined!" << std::endl; break;
		case def::twitch::Message::Type::LEAVE: std::cout << "User " << message.author.name << " has left!" << std::endl; break;
		
		case def::twitch::Message::Type::MESSAGE:
		{
			if (message.text.starts_with("!roll_dice"))
				Reply(message.id, std::to_string(1 + rand() % 6));
		}
		break;

		}

		return true;
	}
};

int main()
{
	MyChat chat;
	chat.Join("def1ni7");
	chat.Start();
	return 0;
}
