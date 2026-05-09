# ft_irc

An IRC server implementing a functional subset of the IRC protocol (RFC 1459), written in C++. Supports multiple simultaneous clients, channels, operator privileges, and channel modes — all over a single non-blocking socket using `poll()`.

Tested with **irssi**, **weechat**, **HexChat**, and **netcat**.

---

## Usage

```
./ircserv <port> <password>
```

```
./ircserv 6667 mypassword
```

- **port** — TCP port to listen on (IRC convention is 6667)
- **password** — clients must send `PASS <password>` before registering

---

## Connecting with irssi

```bash
irssi
/connect localhost 6667 mypassword
/join #channel
/msg OtherUser Hey there!
```

---

## Supported Commands

| Command | Description |
|---------|-------------|
| `PASS` | Authenticate with the server password |
| `NICK` | Set or change nickname |
| `USER` | Set username and real name (completes registration) |
| `PING` | Keepalive — server responds with PONG |
| `JOIN` | Join a channel (creates it if it doesn't exist) |
| `PART` | Leave a channel |
| `PRIVMSG` | Send a message to a channel or user |
| `TOPIC` | Get or set a channel topic |
| `KICK` | Remove a user from a channel (operators only) |
| `INVITE` | Invite a user to an invite-only channel |
| `MODE` | Set or unset channel modes (operators only) |
| `QUIT` | Disconnect from the server |

### Channel Modes (`MODE`)

| Mode | Description |
|------|-------------|
| `+i` / `-i` | Invite-only — only invited users may join |
| `+t` / `-t` | Topic protection — only operators may change topic |
| `+k` / `-k` | Channel key (password) |
| `+o` / `-o` | Grant or revoke operator status |
| `+l` / `-l` | User limit — cap the number of members |

---

## How It Works

**Single-threaded, non-blocking I/O with `poll()`**

The server uses one `poll()` loop to watch all connected file descriptors simultaneously — no threads, no blocking. When a client's fd is readable, incoming data is appended to that client's read buffer. When a complete `\r\n`-terminated line is present, it is parsed and dispatched to the appropriate command handler.

**Client registration state machine**

A client goes through registration states before being allowed to interact:
1. `PASS` must be received first (if server has a password)
2. Both `NICK` and `USER` must be received
3. Only then does the server send welcome numerics (001–004) and mark the client as registered

**Channel model**

Each `Channel` tracks its members, operators, and invited users as pointer sets. It owns its own mode flags, topic, password, and user limit. When a channel becomes empty it is destroyed. Operator actions (KICK, MODE, TOPIC when `+t`) are gated behind an `isOperator()` check.

**Numeric replies**

Responses follow IRC numeric conventions. Two `sendNumeric()` overloads handle single-target and channel-target replies, formatting messages as `:servername NNN nickname <target> :<message>\r\n`.

---

## Structure

| Path | Responsibility |
|------|---------------|
| `src/main.cpp` | Entry point, argument validation, signal handling |
| `src/Server.cpp` | Socket setup, `poll()` loop, command dispatch, client lifecycle |
| `src/Client.cpp` | Per-client state: buffers, identity, registration flags |
| `src/Channel.cpp` | Channel state: members, operators, modes, topic |
| `src/cmds/` | One file per IRC command handler |
| `includes/` | Header files for all three classes |

---

## Building

```
make        # builds ./ircserv
make re     # rebuild from scratch
make fclean # remove objects and binary
```
