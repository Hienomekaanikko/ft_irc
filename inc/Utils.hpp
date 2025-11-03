#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

// IRC reply codes
#define RPL_WELCOME(nick, user, host) (std::string(":localhost 001 ") + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host + "\r\n")
#define RPL_NOTOPIC(nick, channel) (std::string(":localhost 331 ") + nick + " " + channel + " :No topic is set\r\n")
#define RPL_TOPIC(nick, channel, topic) (std::string(":localhost 332 ") + nick + " " + channel + " :" + topic + "\r\n")
#define RPL_NAMREPLY(nick, channel, names) (std::string(":localhost 353 ") + nick + " = " + channel + " :" + names + "\r\n")
#define RPL_ENDOFNAMES(nick, channel) (std::string(":localhost 366 ") + nick + " " + channel + " :End of /NAMES list\r\n")

// IRC error codes
#define ERR_NOSUCHNICK(nick, target) (std::string(":localhost 401 ") + nick + " " + target + " :No such nick/channel\r\n")
#define ERR_NOSUCHCHANNEL(nick, channel) (std::string(":localhost 403 ") + nick + " " + channel + " :No such channel\r\n")
#define ERR_CANNOTSENDTOCHAN(nick, channel) (std::string(":localhost 404 ") + nick + " " + channel + " :Cannot send to channel\r\n")
#define ERR_NONICKNAMEGIVEN(nick) (std::string(":localhost 431 ") + nick + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(nick, badnick) (std::string(":localhost 432 ") + nick + " " + badnick + " :Erroneous nickname\r\n")
#define ERR_NICKNAMEINUSE(nick, badnick) (std::string(":localhost 433 ") + nick + " " + badnick + " :Nickname is already in use\r\n")
#define ERR_USERNOTINCHANNEL(nick, target, channel) (std::string(":localhost 441 ") + nick + " " + target + " " + channel + " :They aren't on that channel\r\n")
#define ERR_NOTONCHANNEL(nick, channel) (std::string(":localhost 442 ") + nick + " " + channel + " :You're not on that channel\r\n")
#define ERR_USERONCHANNEL(nick, target, channel) (std::string(":localhost 443 ") + nick + " " + target + " " + channel + " :is already on channel\r\n")
#define ERR_NOTREGISTERED(nick) (std::string(":localhost 451 ") + nick + " :You have not registered\r\n")
#define ERR_NEEDMOREPARAMS(nick, cmd) (std::string(":localhost 461 ") + nick + " " + cmd + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(nick) (std::string(":localhost 462 ") + nick + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(nick) (std::string(":localhost 464 ") + nick + " :Password incorrect\r\n")
#define ERR_CHANNELISFULL(nick, channel) (std::string(":localhost 471 ") + nick + " " + channel + " :Cannot join channel (+l)\r\n")
#define ERR_INVITEONLYCHAN(nick, channel) (std::string(":localhost 473 ") + nick + " " + channel + " :Cannot join channel (+i)\r\n")
#define ERR_BADCHANNELKEY(nick, channel) (std::string(":localhost 475 ") + nick + " " + channel + " :Cannot join channel (+k)\r\n")
#define ERR_CHANOPRIVSNEEDED(nick, channel) (std::string(":localhost 482 ") + nick + " " + channel + " :You're not channel operator\r\n")

std::vector<std::string> split(const std::string& str, char delimiter);
std::string trim(const std::string& str);
std::string toUpper(const std::string& str);
bool isValidNickname(const std::string& nickname);
bool isValidChannelName(const std::string& name);

#endif
