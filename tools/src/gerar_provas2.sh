#!/usr/bin/env bash

nonce="12345678"
id_rodada="12345678"

i=0
while read linha; do
    id_dispositivo="$(echo "${linha}" | cut -d":" -f1)"
    chave="$(echo "${linha}" | cut -d":" -f2)"
    #./forro-args-dra2 "${chave}" "${nonce}" "${id_rodada}" "${id_dispositivo}" 0 > /dev/null
    echo "${nonce}${id_rodada}${id_dispositivo}${chave}" | sha256sum -t -z > /dev/null
    let i++
    if [ $i -eq 1000 ]; then
        break
    fi
done < id_chave_prova.txt