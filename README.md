# ARP Client

A lightweight command-line tool for sending ARP (Address Resolution Protocol) request packets on a local network.

## Overview

This tool allows you to send custom ARP requests to specified IP addresses over a network interface. 
It's useful for:
- Network debugging and troubleshooting
- Learning about network protocols
- Testing network connectivity
- Discovering MAC addresses of hosts on your network

## Prerequisites

- Linux operating system
- Root privileges (for raw socket operations)
- g++ compiler
- Basic networking libraries

## Building

To build the ARP client:

```bash
make
```

## Usage

Run the tool as root (required for raw socket operations):

```bash
sudo ./arp_client -i <interface> -a <target_ip>
```

Parameters:
- `-i`: Network interface to use (e.g., eth0, wlan0)
- `-a`: Target IPv4 address to send the ARP request to

Example:
```bash
sudo ./arp_client -i eth0 -a 192.168.1.1
```

## Monitoring ARP Responses

To see the ARP replies from the target, you can use tcpdump in another terminal:

```bash
sudo tcpdump -i eth0 proto 0x0806 -XX
```

This command filters for ARP traffic (protocol 0x0806) on the specified interface and displays packet contents in hex format.

## How It Works

The tool performs the following operations:
1. Gets the MAC and IP address of the specified local interface
2. Constructs an Ethernet frame with a broadcast destination 
3. Creates an ARP request packet asking for the MAC address of the target IP
4. Sends the packet via a raw socket
5. Any device with the specified IP will respond with its MAC address (visible via tcpdump)

## Technical Details

The ARP request packet includes:
- Sender's MAC address (from your interface)
- Sender's IP address (from your interface)
- Target MAC address (set to broadcast FF:FF:FF:FF:FF:FF)
- Target IP address (the IP you specified)