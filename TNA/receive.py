#!/usr/bin/env python3
import os
import sys
import subprocess

#Inverte o Endianess
def swap32(valor):
    chunks_invertidos = []
    for i in range(0, len(valor), 4):
        chunk = valor[i:i+4]
        chunk_invertido = chunk[::-1]
        chunks_invertidos.append(chunk_invertido)

    return b''.join(chunks_invertidos)

#Inverte o Endianess
def swap64(valor):
    chunks_invertidos = []
    for i in range(0, len(valor), 8):
        chunk = valor[i:i+8]
        chunk_invertido = chunk[::-1]
        chunks_invertidos.append(chunk_invertido)

    return b''.join(chunks_invertidos)

from scapy.all import (
    UDP,
    get_if_list,
    sniff,
    hexdump,
    bind_layers,
    Ether,
    IP,
    get_if_hwaddr,
    sendp
)

def string_para_bytes_fixos(s, tamanho_em_bits):
    tamanho_em_bytes = tamanho_em_bits // 8  # Convertendo bits para bytes
    # Converter a string para bytes (UTF-8)
    s_bytes = s.encode('utf-8')
    
    # Ajustar o tamanho dos bytes
    if len(s_bytes) < tamanho_em_bytes:
        # Preencher com bytes nulos (\x00) até atingir o tamanho desejado
        s_bytes += b'\x00' * (tamanho_em_bytes - len(s_bytes))
    else:
        # Truncar para o tamanho desejado
        s_bytes = s_bytes[:tamanho_em_bytes]
    
    return s_bytes

from forro_class import ForroState, ForroPayload
from p4dra_class import p4dra_oper, p4dra_reply, p4dra_setup

def get_if():
    ifs=get_if_list()
    iface=None
    for i in get_if_list():
        if "eth0" in i:
            iface=i
            break;
    if not iface:
        print("Cannot find eth0 interface")
        exit(1)
    return iface

def handle_pkt(pkt, iface):
    #if (UDP in pkt and pkt[UDP].dport == 100) or (Ether in pkt and pkt[Ether].type == 0xABCD):
        print("got a packet")
        if (Ether in pkt and pkt[Ether].type == 0x1234):
            pkt.show()
            hexdump(pkt)
            if (pkt[p4dra_oper].oper == 0x1):
                
                chave = "7c8121dd59d29489b0ba66113a107caa4443ad0f1d508cb11b471a9552b06f8d"
                chave_BE = bytes.fromhex(chave)
                chave_LE = swap32(chave_BE)

                id_dispositivo = "30303030303031383430343936393332"
                id_dispositivo_BE = bytes.fromhex(id_dispositivo)
                id_dispositivo_LE = swap32(id_dispositivo_BE)

                conf = ""
                conf_BE = string_para_bytes_fixos(conf, 256)
                conf_LE = swap32(conf_BE)

                id_rodada_LE = pkt[p4dra_oper].id_rodada[:8]
                id_rodada_BE = swap32(id_rodada_LE)

                nonce_LE = pkt[p4dra_oper].id_rodada[8:]
                nonce_BE = swap32(nonce_LE)

                # print(chave_BE.hex(), id_rodada_BE.hex(), nonce_BE.hex(), id_dispositivo_BE.hex(), conf_BE.hex())

                resultado = subprocess.run(["../tools/forro-args-dra2", chave_BE.hex(), id_rodada_BE.hex(), nonce_BE.hex(), id_dispositivo_BE.hex(), conf_BE.hex()], capture_output = True, text=True)
                # print(resultado.stdout)
                prova = bytes.fromhex(resultado.stdout)
                # print("prova:", prova)
                prova_invertida = swap32(prova)
                # print("prova invertida:", prova_invertida)
                oper=b'\x02'
                ethertype = 0x1234
                payload = oper + id_rodada_LE + nonce_LE + id_dispositivo_LE + prova_invertida

                #pkt_resposta = Ether(src=get_if_hwaddr(iface), dst='08:00:00:00:01:01', type=ethertype)
                pkt_resposta = Ether(src=pkt[Ether].dst, dst='08:00:00:00:01:01', type=ethertype)
                pkt_resposta = pkt_resposta / payload
                sendp(pkt_resposta, iface=iface, verbose=True)

            sys.stdout.flush()


def main():
    bind_layers(Ether, ForroPayload, type=0xabcd)
    bind_layers(Ether, p4dra_oper, type=0x1234)
    bind_layers(p4dra_oper, p4dra_reply, oper=0x2)
    bind_layers(p4dra_oper, p4dra_reply, oper=0x4)
    bind_layers(p4dra_oper, p4dra_setup, oper=0x5)
    ifaces = [i for i in os.listdir('/sys/class/net/') if 'eth' in i]
    iface = ifaces[0]
    print("sniffing on %s" % iface)
    sys.stdout.flush()
    sniff(iface = iface,
          prn = lambda x: handle_pkt(x, iface))

if __name__ == '__main__':
    main()
