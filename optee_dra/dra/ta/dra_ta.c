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

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <dra_ta.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "include/forro.h"

/* tamanho do ID_Dispositivo e chave em hexstrings */
const size_t ID_RODADA_SZ_HEX = ID_RODADA_SZ * 2;
const size_t NONCE_SZ_HEX = NONCE_SZ * 2;
const size_t ID_DISPOSITIVO_SZ_HEX = ID_DISPOSITIVO_SZ * 2;
const size_t KEY_SZ_HEX = KEY_SZ * 2;

//Secrets = Chave || ID_DISPOSITIVO
const size_t SECRETS_SZ = KEY_SZ + ID_DISPOSITIVO_SZ;
const size_t SECRETS_SZ_HEX = KEY_SZ_HEX + ID_DISPOSITIVO_SZ_HEX;

// Função para converter uma hexstring para um array de uint8_t
int hexstring_to_byte_array(uint8_t *byte_array, const char *hexstring) {
    size_t len = strlen(hexstring);

    // Calcular o tamanho do array de bytes
    size_t byte_array_len = len / 2;

    // Converter cada par de caracteres hexadecimais para um byte
    for (size_t i = 0; i < byte_array_len; i++) {
        char byte_str[3] = { hexstring[2 * i], hexstring[2 * i + 1], '\0' };
        byte_array[i] = (uint8_t)strtoul(byte_str, NULL, 16);
    }

    return 0;
}

static TEE_Result delete_object(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_Result res;
	char *obj_id;
	size_t obj_id_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	/*
	 * Check object exists and delete it
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_ACCESS_WRITE_META, /* we must be allowed to delete it */
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		TEE_Free(obj_id);
		return res;
	}

	TEE_CloseAndDeletePersistentObject1(object);
	TEE_Free(obj_id);

	return res;
}

static TEE_Result create_raw_object(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_Result res;
	char *obj_id;
	size_t obj_id_sz;
	char *data;
	size_t data_sz;
	uint32_t obj_data_flag;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	data_sz = params[1].memref.size;
	data = TEE_Malloc(data_sz, 0);
	if (!data)
		return TEE_ERROR_OUT_OF_MEMORY;
	TEE_MemMove(data, params[1].memref.buffer, data_sz);

	/*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE |		/* we can later write into the object */
			TEE_DATA_FLAG_ACCESS_WRITE_META |	/* we can later destroy or rename the object */
			TEE_DATA_FLAG_OVERWRITE;		/* destroy existing object of same ID */

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					obj_data_flag,
					TEE_HANDLE_NULL,
					NULL, 0,		/* we may not fill it right now */
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
		TEE_Free(obj_id);
		TEE_Free(data);
		return res;
	}

	res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else {
		TEE_CloseObject(object);
	}
	TEE_Free(obj_id);
	TEE_Free(data);
	return res;
}

static TEE_Result read_raw_object(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_OUTPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	uint32_t read_bytes;
	char *obj_id;
	size_t obj_id_sz;
	char *data;
	size_t data_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	data_sz = params[1].memref.size;
	data = TEE_Malloc(data_sz, 0);
	if (!data)
		return TEE_ERROR_OUT_OF_MEMORY;

	/*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		TEE_Free(obj_id);
		TEE_Free(data);
		return res;
	}

	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
		goto exit;
	}

	if (object_info.dataSize > data_sz) {
		/*
		 * Provided buffer is too short.
		 * Return the expected size together with status "short buffer"
		 */
		params[1].memref.size = object_info.dataSize;
		res = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}

	res = TEE_ReadObjectData(object, data, object_info.dataSize,
				 &read_bytes);
	if (res == TEE_SUCCESS)
		TEE_MemMove(params[1].memref.buffer, data, read_bytes);
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
		EMSG("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u",
				res, read_bytes, object_info.dataSize);
		goto exit;
	}

	/* Return the number of byte effectively filled */
	params[1].memref.size = read_bytes;
exit:
	TEE_CloseObject(object);
	TEE_Free(obj_id);
	TEE_Free(data);
	return res;
}

static TEE_Result reply_proof(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_OUTPUT);
	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;

	uint32_t read_bytes;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	char *secrets_HEX = TEE_Malloc(SECRETS_SZ_HEX, 0);

	size_t obj_id_secrets_sz = (size_t)params[2].memref.size;
	char *obj_id_secrets = TEE_Malloc(obj_id_secrets_sz, 0);
	if (!obj_id_secrets)
		return TEE_ERROR_OUT_OF_MEMORY;
	
	TEE_MemMove(obj_id_secrets, params[2].memref.buffer, obj_id_secrets_sz);

	// Lendo a chave armazenada
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id_secrets, obj_id_secrets_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Falha ao abrir chave no TEE, res=0x%08x", res);
		return res;
	}

	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Falha ao obter metadados do objeto da Chave, res=0x%08x", res);
		goto exit;
	}

	res = TEE_ReadObjectData(object, secrets_HEX, object_info.dataSize,
				 &read_bytes);
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
		EMSG("TEE_ReadObjectData falhou para a chave 0x%08x, read %" PRIu32 " over %u",
				res, read_bytes, object_info.dataSize);
		goto exit;
	}

	// CARREGANDO CHAVE //
	char *chave_HEX = TEE_Malloc(KEY_SZ_HEX, 0);

	// Carregando valores;
	for (int i = 0; i < KEY_SZ_HEX; i++) {
		 chave_HEX[i] = secrets_HEX[i];
	}

	// Alocando Byte Array
	uint8_t *chave = TEE_Malloc(KEY_SZ, 0);

	//Convertendo de Hexstring para Bytes Array
	hexstring_to_byte_array(chave, chave_HEX);

	//Liberando
	// TEE_Free(chave_HEX);

	// Carregando ID Dispositivo //
	char *id_dispositivo_HEX = TEE_Malloc(ID_DISPOSITIVO_SZ_HEX, 0);

	for (int i = 0; i < ID_DISPOSITIVO_SZ_HEX; i++) {
		id_dispositivo_HEX[i] = secrets_HEX[KEY_SZ_HEX + i];
	}

	//Alocando Byte Array
	uint8_t *id_dispositivo = TEE_Malloc(ID_DISPOSITIVO_SZ, 0);

	//Convertendo de Hexstring para Bytes Array
	hexstring_to_byte_array(id_dispositivo, id_dispositivo_HEX);

	//Liberando
	// TEE_Free(id_dispositivo_HEX);
	// TEE_Free(secrets_HEX);

	// Pegando parametros da chamada //
	char *id_rodada_HEX = params[0].memref.buffer;
	char *nonce_HEX = params[1].memref.buffer;
    uint8_t configuracao_rodada[16];
    uint8_t configuracao_dispositivo[16];

	uint8_t *id_rodada = TEE_Malloc(ID_RODADA_SZ, 0);
	uint8_t *nonce = TEE_Malloc(NONCE_SZ, 0);

	hexstring_to_byte_array(id_rodada, id_rodada_HEX);
	hexstring_to_byte_array(nonce, nonce_HEX);

	// Começando o Forro14 //
	stream_ctx input;
    uint8_t *prova = TEE_Malloc(PROOF_SZ, 0);
	uint8_t *configuracao = TEE_Malloc(KEY_SZ, 0);
	
	// Inicializando a chave na matriz de estado
	forro_keysetup(&input, chave);

	// Liberando
	// TEE_Free(chave);

    // id_rodada XOR C0 e C1 (0-3 e 4-7)
    for (int i=0; i < 8; i++) {
        configuracao_rodada[i] = id_rodada[i] ^ configuracao[i];
    }

    // id_dispositivo XOR C2 e C3 (8-11 e 12-15)
    for (int i=0; i < 8; i++) {
        configuracao_dispositivo[i] = id_dispositivo[i] ^ configuracao[i+8];
    }

    // nonce XOR C4 e C5 (16-19 e 20-23)
    for (int i=0; i < 8; i++) {
        configuracao_rodada[i+8] = nonce[i] ^ configuracao[i+16];
    }

    // id_dispositivo XOR C6 e C7 (24-27 e 28-31)
    for (int i=0; i < 8; i++) {
        configuracao_dispositivo[i+8] = id_dispositivo[i+8] ^ configuracao[i+24];
    }

	// Liberando tudo que ja foi usado no calculo da configuracao da rodada.
	// TEE_Free(configuracao);
	// TEE_Free(id_rodada);
	// TEE_Free(nonce);
	// TEE_Free(id_dispositivo);

	// Inicializando a configuracao na matriz de estado
	EMSG("preparando estado inicial (configuracao dispositivo)... 0x%08x", res);
    forro_versetup(&input, configuracao_rodada, configuracao_dispositivo);
	EMSG("estado inicial preparado... Calculando... 0x%08x", res);

	// Inicializando a configuracao na matriz de estado
    forro(prova, input.state);
	EMSG("estado calculado, copiando para o param de saída 0x%08x", res);

	//Devolvendo a prova
	TEE_MemMove(params[3].memref.buffer, prova, PROOF_SZ);
	params[3].memref.size = PROOF_SZ;

	// Liberando
	// TEE_Free(prova);

	EMSG("Finalizando execucao do TA 0x%08x", res);

exit:
	TEE_CloseObject(object);
	TEE_Free(obj_id_secrets);
	return res;
}

TEE_Result TA_CreateEntryPoint(void)
{
	/* Nothing to do */
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	/* Nothing to do */
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
				    TEE_Param __unused params[4],
				    void __unused **session)
{
	/* Nothing to do */
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *session)
{
	/* Nothing to do */
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *session,
				      uint32_t command,
				      uint32_t param_types,
				      TEE_Param params[4])
{
	switch (command) {
	case TA_DRA_CMD_WRITE_RAW:
		return create_raw_object(param_types, params);
	case TA_DRA_CMD_READ_RAW:
		return read_raw_object(param_types, params);
	case TA_DRA_CMD_DELETE:
		return delete_object(param_types, params);
	case TA_DRA_CMD_REQUEST_PROOF:
		return reply_proof(param_types, params);
	default:
		EMSG("Command ID 0x%x is not supported", command);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
