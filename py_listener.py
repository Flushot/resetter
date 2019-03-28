#!/usr/bin/env python
# pip reqs: zmq
import re
import socket

import zmq

cached_hosts = {}


def resolve_ip(ip_addr: str) -> str:
    host = cached_hosts.get(ip_addr)
    if host is None:
        try:
            results = socket.gethostbyaddr(ip_addr)
            if len(results) > 0:
                host = results[0]
        except socket.herror:
            host = ip_addr

        cached_hosts[ip_addr] = host

    return host


def main():
    context = zmq.Context()

    #  Socket to talk to server
    print('Connecting...')
    zmq_sub = context.socket(zmq.SUB)
    zmq_sub.connect('tcp://localhost:5555')
    zmq_sub.setsockopt_string(zmq.SUBSCRIBE, 'reset')

    msg_pat = re.compile(r'^reset ((?:\d{1,3}\.){3}\d{1,3}):(\d+?) ((?:\d{1,3}\.){3}\d{1,3}):(\d+?)$')
    while True:
        message = zmq_sub.recv_string()
        match = msg_pat.match(message)
        if match is not None:
            saddr = resolve_ip(match.group(1))
            sport = int(match.group(2))
            daddr = resolve_ip(match.group(3))
            dport = int(match.group(4))

            print('{}:{} <-> {}:{} was reset'.format(saddr, sport, daddr, dport))


if __name__ == '__main__':
    main()
