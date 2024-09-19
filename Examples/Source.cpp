#include <iostream>
#include <fstream>
#include <string>

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

#include "defTwitch.hpp"

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
	}

protected:
	bool OnUserMessage(const std::string& message_id, const std::string& name, const std::string& text) override
	{
		std::cout << name << ": " << text << std::endl;
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
