#include "Server.hpp"
#include <sstream>
void Server::handleTOPIC(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty()) {
		sendNumeric(client, 461, "TOPIC :Not enough parameters");
		return;
	}
	
	std::string channelName(params[0]);
	auto it = _channels.find(channelName);
	if (it == _channels.end()) {
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	
	Channel &chan = it->second;
	if (!chan.isMember(&client)) {
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}
	
	// If no topic parameter, show current topic
	if (params.size() == 1) {
		const std::string &topic = chan.getTopic();
		if (topic.empty())
			sendNumeric(client, 331, channelName, "No topic is set");
		else
			sendNumeric(client, 332, channelName, topic);
		return;
	}
	
	// Setting topic - check permissions
	if (chan.isTopicProtected() && !chan.isOperator(&client)) {
		sendNumeric(client, 482, channelName + " :You're not channel operator");
		return;
	}
	
	std::string newTopic(params[1]);
	
	std::ostringstream topicStream;
	topicStream << params[1];
	for (size_t i = 2; i < params.size(); ++i) {
		topicStream << " " << params[i];
	}
	newTopic = topicStream.str();
	chan.setTopic(newTopic);
	
	// Broadcast topic change
	std::ostringstream topicMsg;
	topicMsg << ":" << client.getNickname() << "!" 
			 << client.getUsername() << "@" << getClientHost(client.getFd())
			 << " TOPIC " << channelName << " :" << newTopic << "\r\n";
	sendToChannel(chan, topicMsg.str(), nullptr);
}
