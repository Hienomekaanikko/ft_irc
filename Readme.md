# ft_irc

The idea of the project was to create a lightweight IRC server and to implement a functional subset of the IRC protocol (RFC 1459).
This allows multiple clients to communicate in real time.

We used **Irssi** as a reference client.

---

## Features

The mandatory core of an IRC server:

* Multi-client support (with poll() function)
* Nickname management (`NICK`)
* User registration (`USER`)
* Private messages (`PRIVMSG`)
* Channels (`JOIN`, `PART`, channel topic, user lists)
* Setting channel modes (`+i`, `+t`, `+k`, `+o`, `+l`)
* Removing channel modes (`-i`, `-t`, `-k`, `-o`, `-l`)
* Operators (`KICK`, `MODE`)
* Graceful disconnection (`QUIT`)
* Proper numeric replies following IRC conventions (handled with the two different send_numeric() functions for different cases)
* Password protection on server (`PASS`)
* Channel key, invite-only channels
* Ping/Pong handling
* File transfer

---

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/Hienomekeaanikko/ft_irc.git
cd ft_irc
```

### 2. Build the server

```bash
make
```

This produces an executable, called `ircserv`.

---

## Running the server

Input arguments required:

```
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 pass
```

* **port** — Any valid TCP port (usually 6667 for IRC)
* **password** — The password clients must use with the `PASS` command before registering

---

## Connecting with irssi (reference client)

### Start irssi

```bash
irssi
```

### Connect to the server

Inside the irssi prompt:

```bash
/connect localhost port password
```

### Join a channel

```bash
/join #channel
```

### Send a message

Channel message:

```bash
Hello everyone!
```

Private message:

```bash
/msg OtherUser Hey there!
```

---

## Basic IRC workflow (as used by irssi)

When connecting, irssi sends:

```
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
```

ft_irc must wait until both `NICK` and `USER` are received before registering the client and sending the welcome numeric replies.

---

## Notes

* ft_irc is not expected to fully implement the entire IRC specification.
* We tested primarily with **irssi**, but also compatible with other clients (weechat, netcat, HexChat).

---

Well thats about it! Was a fun project for sure and taught a bunch about networking.
