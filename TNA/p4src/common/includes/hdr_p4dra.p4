// Definicao da operacao que sera realizada para definir o proximo parser.
// So fara o parser se o pacote tiver etheretype 0x1234
header p4dra_oper_t {
    bit<8>   oper;   // operacao, 0=start, 1=request e 2=reply. 3 = calcundo verificação, 4 = verificando.
    bit<32>  id_rodada0;
    bit<32>  id_rodada1; 
    bit<32>  id_rodada2; 
    bit<32>  id_rodada3; // Indica o ID da rodada de atestação que o controlador esta solicitando
}

header p4dra_id_dispositivo_t {
    bit<128> id_dispositivo; // ID do dispositivo que enviou a prova
}