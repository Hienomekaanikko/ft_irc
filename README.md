# ft_irc
Internet Relay Chat Server

This project is about creating your own IRC server. You will use an actual IRC client to connect to your server and test it. Internet is ruled by solid standards protocols that allow connected computers to interact with each other. It's always a good thing to know.

## Features

- Full IRC protocol implementation (RFC 1459)
- Non-blocking I/O using `poll()`
- Multi-client support
- Channel management with operators
- User authentication
- Channel modes: invite-only (+i), topic restriction (+t), password (+k), user limit (+l), operator privileges (+o)

## Compilation

```bash
make
```

## Usage

```bash
./ircserv <port> <password>
```

Example:
```bash
./ircserv 6667 mypassword
```

## Supported Commands

- **PASS** - Authenticate with server password
- **NICK** - Set or change nickname
- **USER** - Set user information
- **JOIN** - Join a channel (or create if it doesn't exist)
- **PART** - Leave a channel
- **PRIVMSG** - Send message to user or channel
- **QUIT** - Disconnect from server
- **PING/PONG** - Keep-alive mechanism
- **KICK** - Remove user from channel (operators only)
- **INVITE** - Invite user to channel
- **TOPIC** - View or set channel topic
- **MODE** - Set channel modes (operators only)

## Testing with IRC Client

You can test the server with any IRC client. Example with netcat:

```bash
nc localhost 6667
PASS mypassword
NICK mynickname
USER mynickname 0 * :My Real Name
JOIN #mychannel
PRIVMSG #mychannel :Hello, World!
QUIT
```

Or use a proper IRC client like irssi, weechat, or HexChat.
