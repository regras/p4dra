#Bits
#from scapy.all import Packet, XByteEnumField, XBitField
#class p4dra(Packet):
#    name = "Distributed Remote Attestation Packet"
#    fields_desc=[ XByteEnumField("oper", 0x0, {0: "oper0", 1: "oper1", 2: "oper2", 3: "oper3"}),
#                  XBitField("auth", 0x0, 32),
#                  XBitField("params", 0x0, 80),
#                  XBitField("pad", 0x0, 8)]

#bytes
from scapy.all import Packet, XByteEnumField, StrFixedLenField
class p4dra_oper(Packet):
    name = "Distributed Remote Attestation Operation Header"
    fields_desc=[ XByteEnumField("oper", 0x0, {0: "Start", 1: "Request", 2: "Reply", 3: "oper3"}),
                  StrFixedLenField("id_rodada", "0", 16)]

class p4dra_reply(Packet):
    name = "Distributed Remote Attestation Reply Header"
    fields_desc=[ StrFixedLenField("id_dispositivo", 0, 16),
                  StrFixedLenField("prova", "0", 64)]

class p4dra_setup(Packet):
    name = "Distributed Remote Attestation Setup Header"
    fields_desc=[ StrFixedLenField("chave", "0", 32),
                  StrFixedLenField("id_dispositivo", 0, 16)]