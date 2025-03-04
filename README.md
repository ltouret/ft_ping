# ft_ping

`ft_ping` is a command-line utility that sends ICMP ECHO_REQUEST packets to network hosts, similar to the standard `ping` command. It is a reimplementation of `inetutils 2.0` ping.

## Usage

```sh
./ft_ping [-vq?V] [-c N] [-i N] [-W N] [-e N] [--ttl N] [--help] [--usage] HOST ...
```

## Options

| Option         | Description                                        |
| -------------- | -------------------------------------------------- |
| `--ttl N`      | Specify `N` as the time-to-live value for packets. |
| `-c N`         | Stop after sending `N` packets.                    |
| `-i N`         | Wait `N` seconds between sending each packet.      |
| `-e N`         | Define an identifier  `N` for the ping session.    |
| `-W N`         | Wait up to `N` seconds for a response.             |
| `-v`           | Enable verbose output.                             |
| `-q`           | Enable quiet output.                               |
| `-?`, `--help` | Show the help message.                             |
| `--usage`      | Display a short usage message.                     |
| `-V`           | Print program version.                             |

## Example Usage

Send 5 ping requests to `example.com` with a 1-second interval:

```sh
./ft_ping -c 5 -i 1 example.com
```

Enable verbose output while pinging `example.com`:

```sh
./ft_ping -v example.com
```

Wait up to 5 seconds for a response:

```sh
./ft_ping -W 5 example.com
```

## Implementation Detail

The socket used for sending ICMP packets is created as follows:

```c
socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
```

### Why `SOCK_DGRAM`?

Unlike `SOCK_RAW`, which requires root privileges, `SOCK_DGRAM` allows sending ICMP Echo Requests without sudo. This means:

* The kernel automatically fills in necessary fields like the ICMP checksum.

* ICMP Echo Requests are properly routed without direct raw socket manipulation.

* The program can function with normal user permissions, making it more accessible and secure.

## Compilation

To compile the project, use the provided `Makefile`:

```sh
make
```

You can then use `./ft_ping` to start sending ICMP packets.

## License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**.  
You are free to use, modify, and redistribute it under the same license.

For the full text of the GPLv3, visit:  
[https://www.gnu.org/licenses/gpl-3.0.html](https://www.gnu.org/licenses/gpl-3.0.html).

## Author

Developed by LT.