# Resetter

Denial of service tool that will actively terminate and prevent TCP connections to/from an IP address and/or port.

To achieve this, a number of methods are used:

* [TCP reset attack](https://en.wikipedia.org/wiki/TCP_reset_attack)
* [ARP poisoning](https://en.wikipedia.org/wiki/ARP_spoofing)

## Requirements

* Libraries
    * libnet 1.1
    * libpcap
    * cunit
* Tools
    * cmake 3.13+
