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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* TA API: UUID and command IDs */
#include <dra_ta.h>

/* tamanho do ID_Dispositivo e chave em hexstrings */
const size_t ID_RODADA_SZ_HEX = ID_RODADA_SZ * 2;
const size_t NONCE_SZ_HEX = NONCE_SZ * 2;
const size_t ID_DISPOSITIVO_SZ_HEX = ID_DISPOSITIVO_SZ * 2;
const size_t KEY_SZ_HEX = KEY_SZ*2;

//Secrets = Chave || ID_DISPOSITIVO
const size_t SECRETS_SZ = KEY_SZ + ID_DISPOSITIVO_SZ;
const size_t SECRETS_SZ_HEX = KEY_SZ_HEX + ID_DISPOSITIVO_SZ_HEX;

/* TEE resources */
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx)
{
	TEEC_UUID uuid = TA_DRA_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

TEEC_Result read_secure_object(struct test_ctx *ctx, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_DRA_CMD_READ_RAW,
				 &op, &origin);
	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_SHORT_BUFFER:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command READ_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result write_secure_object(struct test_ctx *ctx, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_DRA_CMD_WRITE_RAW,
				 &op, &origin);

	if (res != TEEC_SUCCESS)
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);

	switch (res) {
	case TEEC_SUCCESS:
		break;
	default:
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result request_proof(struct test_ctx *ctx, char *prova, size_t prova_sz, char *id_rodada, char *nonce)
{
	// Estrutura da operacao a ser executada
	TEEC_Operation op;

	// Guarda a origem de resposta de um chamada feita (origens como SMC, TA, etc)
	uint32_t origin;

	// Estrutura do resultado da chamada a API
	TEEC_Result res;

	// Tamanhos dos buffers a serem utilizados (id_rodada e nonce)
	size_t id_rodada_sz = strlen(id_rodada);
	size_t nonce_sz = strlen(nonce);
	size_t secrets_sz = strlen(ID_OBJ_SECRETS);

	// Parametros = Entrada, Entrada, Saída, None
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT);
	
	// Definindo parametro 0 como id_rodada
	op.params[0].tmpref.buffer = id_rodada;
	op.params[0].tmpref.size = id_rodada_sz;

	// Definindo parametro 1 como nonce
	op.params[1].tmpref.buffer = nonce;
	op.params[1].tmpref.size = nonce_sz;

	// Definindo parametro 2 como object_id dos secrets
	op.params[2].tmpref.buffer = ID_OBJ_SECRETS;
	op.params[2].tmpref.size = secrets_sz;

	// Definindo parametro 3 como prova
	op.params[3].tmpref.buffer = prova;
	op.params[3].tmpref.size = prova_sz;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_DRA_CMD_REQUEST_PROOF,
				 &op, &origin);
	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_SHORT_BUFFER:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command request_proof failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result delete_secure_object(struct test_ctx *ctx, char *id)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_DRA_CMD_DELETE,
				 &op, &origin);

	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command DELETE failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

int main(int argc, char *argv[])
{
	struct test_ctx ctx;

	TEEC_Result res;

	char key[KEY_SZ_HEX];
	uint8_t *obj_id_chave = ID_OBJ_KEY;

	char id_dispositivo[ID_DISPOSITIVO_SZ_HEX];
	uint8_t *obj_id_dispositivo = ID_OBJ_DISP;

	char id_rodada[ID_RODADA_SZ_HEX];
	uint8_t nonce[NONCE_SZ_HEX];
	
	char secrets[SECRETS_SZ_HEX];
	char *obj_id_secrets = ID_OBJ_SECRETS;

	uint8_t prova[PROOF_SZ];

	int comando = atoi(argv[1]);
	prepare_tee_session(&ctx);

	// reading the provided key
	switch (comando) {
		case TA_DRA_CMD_READ_RAW:
			printf("- Lendo o conteudo no storage seguro\n");
			res = read_secure_object(&ctx, obj_id_chave,
					key, KEY_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao ler chave do storage seguro");

			printf("key: %p, key: %.*s\n", key, KEY_SZ_HEX, key);

			res = read_secure_object(&ctx, obj_id_dispositivo,
					id_dispositivo, ID_DISPOSITIVO_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao ler ID_Dispositivo do storage seguro");

			printf("id_dispositivo: %p, id_dispositivo: %.*s\n", id_dispositivo, ID_DISPOSITIVO_SZ_HEX, id_dispositivo);

			res = read_secure_object(&ctx, obj_id_secrets,
					secrets, SECRETS_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao ler os secrets do storage seguro");
			
			printf("secrets: %p, secrets: %.*s\n", secrets, SECRETS_SZ_HEX, secrets);
			break;
		case TA_DRA_CMD_WRITE_RAW:
			printf("- gravando o conteudo no storage seguro\n");

			// Obtendo argumentos: chave e id_dispositivo
			memcpy(key, argv[2], KEY_SZ_HEX);
			memcpy(id_dispositivo, argv[3], ID_DISPOSITIVO_SZ_HEX);

			// Concatenando hexstrings em um "Secrets"
			for (int i = 0; i < KEY_SZ_HEX; i++) {
				secrets[i] = key[i];
			}

			for (int i = 0; i < ID_DISPOSITIVO_SZ_HEX; i++) {
				secrets[KEY_SZ_HEX + i] = id_dispositivo[i];
			}

			// Escrevendo hexstrings no Storage seguro do TA //
			res = write_secure_object(&ctx, obj_id_chave,
						key, KEY_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao salvar a Chave no storage seguro");

			res = write_secure_object(&ctx, obj_id_dispositivo,
						id_dispositivo, ID_DISPOSITIVO_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao salvar o ID_Dispositivo no storage seguro");	

			res = write_secure_object(&ctx, obj_id_secrets,
						secrets, SECRETS_SZ_HEX);
			if (res != TEEC_SUCCESS)
				errx(1, "Falha ao salvar o Secrets no storage seguro");	
		
			printf("Conteudo gravado no storage seguro!")
			break;
		case TA_DRA_CMD_DELETE:
			break;
		case TA_DRA_CMD_REQUEST_PROOF:
			// Recebendo argumentos: id_rodada e nonce da rodada
			memcpy(id_rodada, argv[2], ID_RODADA_SZ_HEX);
			memcpy(nonce, argv[3], NONCE_SZ_HEX);

			// Solicitando prova ao TA
			request_proof(&ctx, prova, PROOF_SZ, id_rodada, nonce);

			// Imprimindo hexstring da prova
			for (int i = 0; i < PROOF_SZ; i++) { // percorre 64 bytes
			 	printf("%02x", prova[i]); // Produz 128 caracteres de hexstring
			}
			break;
		default:
			printf("comando nao reconhecido: %s", argv[0]);
			return 1;
			break;
	}

	terminate_tee_session(&ctx);
	return 0;
}
