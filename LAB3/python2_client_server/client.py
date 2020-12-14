#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Echo klientas
import socket
import sys

# Pirma paleisti server.py su chosen port number,
# ir tada client.py su tuo paciu port number.

hostas = '127.0.0.1' # Kur yra mūsų serveris

portas = int(input('Įvesk porto numerį (>1024) --> '))  # Turi būti tas pats kaip ir serverio
s = None
for rezultatas in socket.getaddrinfo(hostas, portas, socket.AF_UNSPEC, socket.SOCK_STREAM):
    print(40 * "-")
    print("res: ", rezultatas)
    # res: (2, 1, 6, '', ('127.0.0.1', 10000))

    adresu_seima, soketo_tipas, protokolas, canoninis_vardas, soketo_adresas = rezultatas

    print('adresu_seima: %s' % adresu_seima)
    print('soketo_tipas: %s' % soketo_tipas)
    print('protokolas: %s' % protokolas)
    print('canoninis_vardas: %s' % canoninis_vardas)
    print('soketo_adresas:'),
    print(soketo_adresas)
    # adresu_seima: 2
    # soketo_tipas: 1
    # protokolas: 6
    # canoninis_vardas: ''
    # soketo_adresas: ('127.0.0.1', {portas})
    print(40 * "-")
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

print(40 * "-")
print('Gauta: ', repr(gavome))

print('Gauta: '),
print(repr(gavome))

print(gavome)
