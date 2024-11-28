#!/usr/bin/env bash

nonce="3132333435363738"
id_rodada="3132333435363738"

for i in $(seq 1 1000000); do
#for i in $(seq 1 10); do
    id_dispositivo="$(printf "%07d" ${i})$(date +%N)"
    id_dispositivo="$(for i in $(seq 0 15); do echo -n "3${id_dispositivo:${i}:1}"; done)"
    chave="$(echo "${nonce}:${id_dispositivo}:${id_rodada}" | sha256sum -t -z | cut -d" " -f1)"
    prova="$(../forro-args-dra2 "${chave}" "${nonce}" "${id_rodada}" "${id_dispositivo}" 0)"
    echo "${id_dispositivo}:${chave}:${prova}:"
done > ../id_chave_prova_1m.txt

tail -n1000 ../id_chave_prova_1m.txt > ../id_chave_prova_1k.txt
tail -n200 ../id_chave_prova_1k.txt > ../id_chave_prova_200.txt
tail -n1 ../id_chave_prova_200.txt > ../id_chave_prova_1.txt