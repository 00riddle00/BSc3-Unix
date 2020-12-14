#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Echo klientas
import socket
import sys

# Naudoti kaip standalone client80.py, tiesiog kreiptis
# i koki nors egzistuojanti domena/IP.

# pvz. www.google.com, arba IP adresas
hostas = raw_input('Įvesk adresą --> ') # Kur yra mūsų serveris

portas = 80  # Turi būti tas pats kaip ir serverio
s = None
for rezultatas in socket.getaddrinfo(hostas, portas, socket.AF_UNSPEC, socket.SOCK_STREAM):
    adresu_seima, soketo_tipas, protokolas, canoninis_vardas, soketo_adresas = rezultatas
    try:
        s = socket.socket(adresu_seima, soketo_tipas, protokolas)
    except OSError as msg:
        s = None
        continue
    try:
        s.connect(soketo_adresas)
    except OSError as msg:
        s.close()
        s = None
        continue
    break
if s is None:
    print('Negaliu atidaryti soketo  :(')
    sys.exit(1)
print('Įvesk eilutę siuntimui --> '),
siuntimui = sys.stdin.readline()
s.sendall(siuntimui)
gavome = s.recv(1024)
s.close()
# Gausime kazkoki atsaka, pvz 404 Not Found, etc.
print('Gauta ...', repr(gavome))
