from netaddr import IPAddress

#Arquivo IDs e chaves
file_path = "../../data/id_chave_prova_20k.txt"
#Versão do Forro
forro = 14
#Total rodadas (ou total de travessias)
total_rodadas = (forro * 4) + 1
#Portas do cliente (provador) e da CPU (controlador)
##TOFINO-MODEL
#h1: 0
#h2: 1
porta_cliente = 0
porta_CPU = 1
#Quantidade de dispositivos que o switch pode verificar
quantidade_dispositivos = 20000

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

#Le o arquivo formato id_dispositivo:chave:prova e extrai id_dispositivo e chave[0..7] para inserir registros nas tabelas
def process_file(file_path, quantidade):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    lines = lines[:quantidade]
    results = []

    for line in lines:
        # Remover espaços em branco ou quebras de linha
        line = line.strip()

        # Dividir a linha pelos dois pontos ':'
        parts = line.split(':')

        # Primeiro campo (decodificar como hexadecimal de 128 bits)
        id_dispositivo_str = parts[0]
        # Converter para hexadecimal
        #id_dispositivo_hex = "0x" + id_dispositivo_str
        id_dispositivo_hex = id_dispositivo_str

        # Segundo campo (interpretar como hexadecimal e dividir em 8 partes)
        chave_hex = parts[1]

        # Dividir em 8 partes de 8 dígitos hexadecimais
        #chave_parts = [f"0x{chave_hex[i:i+8]}" for i in range(0, len(chave_hex), 8)]
        chave_parts = [f"{chave_hex[i:i+8]}" for i in range(0, len(chave_hex), 8)]

        # Armazenar resultados
        results.append({
            'id_dispositivo': id_dispositivo_hex,
            'chave': chave_parts
        })

    return results

p4 = bfrt.p4dra.pipe

# This function can clear all the tables and later on other fixed objects
# once bfrt support is added.
def clear_all(verbose=True, batching=True):
    global p4
    global bfrt

    # The order is important. We do want to clear from the top, i.e.
    # delete objects that use other objects, e.g. table entries use
    # selector groups and selector groups use action profile members

    for table_types in (['MATCH_DIRECT', 'MATCH_INDIRECT_SELECTOR'],
                        ['SELECTOR'],
                        ['ACTION_PROFILE']):
        for table in p4.info(return_info=True, print_info=False):
            if table['type'] in table_types:
                if verbose:
                    print("Clearing table {:<40} ... ".
                          format(table['full_name']), end='', flush=True)
                table['node'].clear(batch=batching)
                if verbose:
                    print('Done')

clear_all(verbose=True)

#Inserindo regras de calculo do Forro
for rodada in range(1, total_rodadas):
    #Rodadas ímpares é Ingress, Rodadas pares é Egress
    if rodada % 2 == 0:
        pipeline = "Egress"
        table = "eg"
        action = "e"
    else:
        pipeline = "Ingress"
        table = "ig"
        action = "i"

    #O QR a ser executado na travessia varia de 0 a 7, pares no Ingress e Ímpares no Egress
    qr = (rodada - 1) % 8

    #Último QR tem uma ação específicas para a finalizacao, estágio 11
    if rodada == (total_rodadas - 1):
        limite = 11
    else:
        limite = 12

    #Gerando codigo python para inserir registro na tabela
    for i in range(0, limite):
        code = f"p4.{pipeline}.tbl_forro_{table}{i}.add_with_{action}{i}_qr{qr}(rodada={rodada})"
        exec(code)

#Inserindo regra para encaminhar para finalizacao
p4.Egress.tbl_forro_eg11.add_with_e11_qr7_fin(rodada=total_rodadas-1)

# Parametros para teste
conf_ini = 0x0

id_chaves = process_file(file_path, quantidade_dispositivos)

# Percorrendo IDs e chaves para inicializar
for i, linha in enumerate(id_chaves):
    id_dispositivo_ini = int(inverter_endianess(linha['id_dispositivo']),16)
    for i, parte in enumerate(linha['chave']):
        parte = int(inverter_endianess(parte),16)
        exec(f"key_{i} = {parte}")

    # Calculando C XOR id_dispositivo
    conf_param = conf_ini ^ ( ( ( (id_dispositivo_ini & 0xFFFFFFFFFFFFFFFF0000000000000000) >> 64) << 128 ) | (id_dispositivo_ini & 0xFFFFFFFFFFFFFFFF) )

    # Adicionando registros para o P4DRA iniciar calculo e verificar prova para cada dispositivo
    p4.Ingress.tbl_p4dra_ig0.add_with_carregar_estado_0(oper=0x2,id_dispositivo=id_dispositivo_ini,configuracao=conf_param,key0=key_0,key1=key_1,key2=key_2,key3=key_3,key4=key_4,key5=key_5,key6=key_6,key7=key_7)
    p4.Ingress.tbl_p4dra_ig0_fin.add_with_finalizar_forro_0(id_dispositivo=id_dispositivo_ini,configuracao=conf_param,key0=key_0,key1=key_1,key2=key_2,key3=key_3,key4=key_4,key5=key_5,key6=key_6,key7=key_7)

### Regras Padrao ###
#Encaminha pacotes de Start como Request
p4.Ingress.tbl_p4dra_ig0.add_with_forward_request(oper=0x0,port=porta_cliente) #pode ser usado id_dispositivo para indicar a porta de saida por dispositivo
#Encaminha pacotes de Setup
p4.Ingress.tbl_p4dra_ig0.add_with_forward(oper=0x5) 

# Trilha de inicializacao
p4.Ingress.tbl_p4dra_ig1.add_with_carregar_estado_1(oper=0x2) #Dummy, mas nao pode entrar na default action por usar hash.get()
p4.Ingress.tbl_p4dra_ig2.add_with_carregar_estado_2(oper=0x2) #Dummy, mas nao pode entrar na default action por usar hash.get()
p4.Ingress.tbl_p4dra_ig3.add_with_carregar_estado_3_iniciar(oper=0x2) #Dummy, mas nao pode entrar na default action por usar hash.get()

# Trilha de finalizacao
p4.Ingress.tbl_p4dra_ig1_fin.add_with_finalizar_forro_1(oper=0x4) #Dummy, mas nao pode entrar na default action por usar hash.get()
p4.Ingress.tbl_p4dra_ig2_fin.add_with_finalizar_forro_2(oper=0x4) #Dummy, mas nao pode entrar na default action por usar hash.get()
p4.Ingress.tbl_p4dra_ig3_fin.add_with_finalizar_forro_3(oper=0x4) #Dummy, mas nao pode entrar na default action por usar hash.get()

# Decide se vai fazer a verificacao ou nao
#p4.Ingress.tbl_p4dra_ig4_fin.add_with_finalizar_forro_4_enviar(oper=0x4,port=porta_CPU) #pular verificacao
p4.Ingress.tbl_p4dra_ig4_fin.add_with_finalizar_forro_4(oper=0x4,port=porta_CPU) #Define a porta para provas invalidas. fazer verificacao

# Trilha de verificacao
p4.Ingress.tbl_p4dra_ig5_fin.add_with_verificar_prova_0(oper=0x4) #Dummy, adicionando por adicionar. Já está na Default
p4.Ingress.tbl_p4dra_ig6_fin.add_with_verificar_prova_0(verificacao0=0x0,verificacao1=0x0,verificacao2=0x0,verificacao3=0x0) # Se prova valida, entao tudo = 0x0
p4.Ingress.tbl_p4dra_ig7_fin.add_with_verificar_prova_1(verificacao0=0x0,verificacao1=0x0,verificacao2=0x0,verificacao3=0x0) # Se prova valida, entao tudo = 0x0
p4.Ingress.tbl_p4dra_ig8_fin.add_with_NoAction(verificacao0=0x0,verificacao1=0x0,verificacao2=0x0) # Se prova valida, entao tudo = 0x0
p4.Egress.tbl_p4dra_eg0_fin.add_with_verificar_prova(oper=0x4) #Dummy, adicionando por adicionar. Já está na Default
p4.Egress.tbl_p4dra_eg1_fin.add_with_NoAction(verificacao=0x0) # Se prova valida, entao tudo = 0x0

#######

#Inserindo alteracoes
bfrt.complete_operations()
