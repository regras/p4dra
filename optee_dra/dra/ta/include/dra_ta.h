/*
 * Copyright (c) 2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __DRA_H__
#define __DRA_H__

/* UUID of the trusted application */
#define TA_DRA_UUID \
		{ 0xc3b0ff51, 0x689e, 0x40db, \
			{ 0x83, 0x95, 0xc2, 0xdd, 0x5b, 0xc2, 0xf3, 0xdb } }
/*
 * TA_DRA_CMD_READ_RAW - Create and fill a secure storage file
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Raw data dumped from the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_DRA_CMD_READ_RAW		0

/*
 * TA_DRA_CMD_WRITE_RAW - Create and fill a secure storage file
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Raw data to be writen in the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_DRA_CMD_WRITE_RAW		1

/*
 * TA_DRA_CMD_DELETE - Delete a persistent object
 * param[0] (memref) ID used the identify the persistent object
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_DRA_CMD_DELETE		2

/*
 * TA_DRA_CMD_REQUEST_PROOF - Solicita a prova
 * param[0] (memref) ID da rodada
 * param[1] (memref) Nonce da rodada
 * param[2] (memref) Prova calculada
 * param[3] unused
 */
#define TA_DRA_CMD_REQUEST_PROOF	3

/* ID dos objetos no TEE guardando o id_dispositivo e a chave */
#define ID_OBJ_DISP "1"
#define ID_OBJ_KEY "2"
#define ID_OBJ_SECRETS "3"

/* tamanho do ID_Dispositivo e chave em Bytes */
#define ID_RODADA_SZ 8 // 64 bits = 8 bytes = 16 chars hexstring
#define NONCE_SZ 8 // 64 bits = 8 bytes = 16 chars hexstring
#define ID_DISPOSITIVO_SZ 16 // 128 bits = 16 bytes = 32 chars hexstring
#define KEY_SZ 32 //256 bits = 32 bytes = 64 chars hexstring
#define PROOF_SZ 64 // 512 bits = 64 bytes = 128 chars hexstring

#endif /* __DRA_H__ */
