#! /bin/bash

iptables -t nat -F
iptables -t nat -X

iptables -t nat -A OUTPUT -m state --state NEW -p tcp -d 127.0.0.1 --dport 8024 -j DNAT --to-destination 127.0.0.1:80
iptables -t nat -A OUTPUT -m state --state NEW -p tcp -d 127.0.0.1 --dport 8001 -j DNAT --to-destination 127.0.0.1:80
