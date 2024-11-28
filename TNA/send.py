#!/usr/bin/env python3
import random
import socket
import sys
import binascii
import struct

from scapy.all import IP, UDP, Ether, get_if_hwaddr, get_if_list, sendp

from forro_class import ForroState, ForroPayload
from p4dra_class import p4dra_oper, p4dra_reply

#Inverte o Endianess
def swap32(valor):
    chunks_invertidos = []
    for i in range(0, len(valor), 4):
        chunk = valor[i:i+4]
        chunk_invertido = chunk[::-1]
        chunks_invertidos.append(chunk_invertido)

    return b''.join(chunks_invertidos)
        

#Inverte o Endianess da hexstring informada
def inverter_endianess(input_hex):
    # Verifica se o comprimento da string é múltiplo de 8 (4 bytes)
    if len(input_hex) % 8 != 0:
        raise ValueError("A entrada deve ter um comprimento múltiplo de 8 caracteres (4 bytes).")

    # Divide a string em blocos de 8 caracteres (4 bytes)
    blocks = [input_hex[i:i+8] for i in range(0, len(input_hex), 8)]

    # Inverte a endianness em cada bloco de 4 bytes
    inverted_blocks = [''.join(reversed([block[j:j+2] for j in range(0, 8, 2)])) for block in blocks]

    # Junta todos os blocos invertidos em uma string final
    result = ''.join(inverted_blocks)

    return result

def get_if():
    ifs=get_if_list()
    iface=None # "h1-eth0"
    for i in get_if_list():
        if "eth" in i:
            iface=i
            break;
    if not iface:
        print("Cannot find eth0 interface")
        exit(1)
    return iface

def padding(msg, tamanho):
    #Limita o tamanho da mensagem a 64 caracteres, 512 bits
    comprimento_msg = tamanho if len(msg) > tamanho else len(msg)
    #Converte a mensagem para bytes encodando ela
    mensagem_bytes = msg[:comprimento_msg].encode("utf-8")
    #Calcula quantos zeros faltam para 512 bits e preenche o resto da string
    mensagem_com_pad = mensagem_bytes + b'\x00' * (tamanho - comprimento_msg)

    return mensagem_com_pad

def main():
    if len(sys.argv)<2:
        print('pass 2 arguments: destination "<id_rodada>"')
        exit(1)

    #Carregando argumentos, ja invertendo o Endianess do nonce, key e mensagem
    addr = socket.gethostbyname(sys.argv[1])
    tipo = sys.argv[2]
    if tipo == "forro":
        if len(sys.argv)<3:
            print('pass 3 arguments: destination "forro" "<payload>"')
            exit(1)
        payload = swap32(padding(sys.argv[3], 64))
        ethertype = 0xabcc
    elif tipo == "start":
        if len(sys.argv)<3:
            print('pass 3 arguments: destination "start" "<id_rodada>"')
            exit(1)
        id_rodada = bytes.fromhex(inverter_endianess(sys.argv[3]))
        oper=b'\x00' #Operacao do p4DRA, 0=start, 1=request, 2=reply
        ethertype = 0x1234
        payload = oper + id_rodada
    elif tipo == "request":
        if len(sys.argv)<4:
            print('pass 4 arguments: destination "request" "<id_rodada>"')
            exit(1)
        id_rodada = bytes.fromhex(inverter_endianess(sys.argv[3]))
        oper=b'\x01'
        ethertype = 0x1234
        payload = oper + id_rodada
    elif tipo == "reply":
        if len(sys.argv)<5:
            print('pass 5 arguments: destination "reply" "<id_rodada>" "<id_dispositivo>" "<prova>"')
            exit(1)
        id_rodada = bytes.fromhex(inverter_endianess(sys.argv[3]))
        id_dispositivo = bytes.fromhex(inverter_endianess(sys.argv[4]))
        prova = bytes.fromhex(inverter_endianess(sys.argv[5]))
        oper=b'\x02'
        ethertype = 0x1234
        payload = oper + id_rodada + id_dispositivo + prova
    elif tipo == "setup":
        if len(sys.argv)<4:
            print('pass 4 arguments: destination "setup" "<chave>" "<id_dispositivo>"')
            exit(1)
        oper=b'\x05'
        id_rodada = padding("", 8)
        chave = padding(sys.argv[3], 32)
        id_dispositivo = padding(sys.argv[4], 16)
        ethertype = 0x1234
        payload = oper + id_rodada + chave + id_dispositivo
    else:
        print('"tipo" invalid: tipo="forro|start|request|reply|setup"')
        exit(1)

    iface = get_if()
    print(payload)

    print("sending on interface %s to %s" % (iface, str(addr)))
    pkt = Ether(src=get_if_hwaddr(iface), dst='08:00:00:00:01:02', type=ethertype)
    #pkt = Ether(src=get_if_hwaddr(iface), dst='52:54:00:00:00:01', type=ethertype)
    pkt = pkt / payload

    sendp(pkt, iface=iface, verbose=False)

if __name__ == '__main__':
    main()
