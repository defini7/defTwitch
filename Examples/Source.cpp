#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../defTwitch.hpp"

const std::string INFO = "website: defini7.github.io";

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

void SplitString(std::string_view buffer, std::vector<std::string>& output)
{
	output.clear();

	std::string token;

	for (auto c : buffer)
	{
		if (c == ' ' && !token.empty())
		{
			output.push_back(token);
			token.clear();
		}
		else
			token.append(1, c);
	}

	if (!token.empty())
		output.push_back(token);
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
		using Type = def::twitch::Message::Type;

		switch (message.type)
		{
		case Type::JOIN: std::cout << "User " << message.author.name << " has joined!" << std::endl; break;
		case Type::LEAVE: std::cout << "User " << message.author.name << " has left!" << std::endl; break;
		
		case Type::MESSAGE:
		{
			if (message.text.starts_with("!roll_dice"))
				Reply(message.id, std::to_string(1 + rand() % 6));

			if (message.text.starts_with("!when"))
			{
				SplitString(message.text, buffer);

				if (buffer.size() > 1)
					Reply(message.id, buffer[1] + " is tomorrow!");
			}

			if (message.text.starts_with("!ban"))
			{
				SplitString(message.text, buffer);

				if (buffer.size() > 1)
					Reply(message.id, buffer[1] + " has been banned!");
			}

			if (message.text.starts_with("!rr"))
			{
				SplitString(message.text, buffer);

				if (buffer.size() > 1)
					Reply(message.id, buffer[1] + " must be rewritten in rust!");
			}

			if (message.text.starts_with("!info"))
				Reply(message.id, INFO);
		}
		break;

		}

		return true;
	}

protected:
	std::vector<std::string> buffer;

};

int main()
{
	MyChat chat;
	chat.Join("def1ni7");
	chat.Start();
	return 0;
}
