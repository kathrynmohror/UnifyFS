/*
 * Copyright (c) 2019, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * Copyright 2017-2019, UT-Battelle, LLC.
 *
 * LLNL-CODE-741539
 * All rights reserved.
 *
 * This is the license for UnifyCR.
 * For details, see https://github.com/LLNL/UnifyCR.
 * Please read https://github.com/LLNL/UnifyCR/LICENSE for full license text.
 */

#include "margo_server.h"
#include "unifycr_global.h"

// global variables
ServerRpcContext_t* unifycrd_rpc_context;
bool margo_use_tcp = true;

static const char* PROTOCOL_MARGO_SHM   = "na+sm://";
static const char* PROTOCOL_MARGO_VERBS = "ofi+verbs://";
static const char* PROTOCOL_MARGO_TCP   = "ofi+tcp://";

/* setup_ofi_target - Initializes the libfabrics margo target */
static margo_instance_id setup_ofi_target(void)
{
    /* initialize margo */
    hg_return_t hret;
    hg_addr_t addr_self;
    char self_string[128];
    hg_size_t self_string_sz = sizeof(self_string);
    margo_instance_id mid;
    const char* margo_protocol;

    if (margo_use_tcp) {
        margo_protocol = PROTOCOL_MARGO_TCP;
    } else {
        margo_protocol = PROTOCOL_MARGO_VERBS;
    }

    mid = margo_init(margo_protocol, MARGO_SERVER_MODE, 1, 1);
    if (mid == MARGO_INSTANCE_NULL) {
        LOGERR("margo_init(%s)", margo_protocol);
        return mid;
    }

    /* figure out what address this server is listening on */
    hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        LOGERR("margo_addr_self()");
        margo_finalize(mid);
        return MARGO_INSTANCE_NULL;
    }
    hret = margo_addr_to_string(mid,
                                self_string, &self_string_sz,
                                addr_self);
    if (hret != HG_SUCCESS) {
        LOGERR("margo_addr_to_string()");
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return MARGO_INSTANCE_NULL;
    }
    LOGDBG("ofi margo RPC server: %s", self_string);
    margo_addr_free(mid, addr_self);

    /* publish rpc address of server for remote servers */
    // TODO

    return mid;
}

static void register_server_server_rpcs(margo_instance_id mid)
{

}

/* setup_sm_target - Initializes the shared-memory margo target */
static margo_instance_id setup_sm_target(void)
{
    /* initialize margo */
    hg_return_t hret;
    hg_addr_t addr_self;
    char self_string[128];
    hg_size_t self_string_sz = sizeof(self_string);
    margo_instance_id mid;

    mid = margo_init(PROTOCOL_MARGO_SHM, MARGO_SERVER_MODE, 1, 1);
    if (mid == MARGO_INSTANCE_NULL) {
        LOGERR("margo_init(%s)", PROTOCOL_MARGO_SHM);
        return mid;
    }

    /* figure out what address this server is listening on */
    hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        LOGERR("margo_addr_self()");
        margo_finalize(mid);
        return MARGO_INSTANCE_NULL;
    }
    hret = margo_addr_to_string(mid,
                                self_string, &self_string_sz,
                                addr_self);
    if (hret != HG_SUCCESS) {
        LOGERR("margo_addr_to_string()");
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return MARGO_INSTANCE_NULL;
    }
    LOGDBG("shared-memory margo RPC server: %s", self_string);
    margo_addr_free(mid, addr_self);

    /* publish rpc address of server for local clients */
    rpc_publish_server_addr(self_string);

    return mid;
}

static void register_client_server_rpcs(margo_instance_id mid)
{
    /* register client-server RPCs */
    MARGO_REGISTER(mid, "unifycr_mount_rpc",
                   unifycr_mount_in_t, unifycr_mount_out_t,
                   unifycr_mount_rpc);

    MARGO_REGISTER(mid, "unifycr_unmount_rpc",
                   unifycr_unmount_in_t, unifycr_unmount_out_t,
                   unifycr_unmount_rpc);

    MARGO_REGISTER(mid, "unifycr_metaget_rpc",
                   unifycr_metaget_in_t, unifycr_metaget_out_t,
                   unifycr_metaget_rpc);

    MARGO_REGISTER(mid, "unifycr_metaset_rpc",
                   unifycr_metaset_in_t, unifycr_metaset_out_t,
                   unifycr_metaset_rpc);

    MARGO_REGISTER(mid, "unifycr_fsync_rpc",
                   unifycr_fsync_in_t, unifycr_fsync_out_t,
                   unifycr_fsync_rpc);

    MARGO_REGISTER(mid, "unifycr_filesize_rpc",
                   unifycr_filesize_in_t, unifycr_filesize_out_t,
                   unifycr_filesize_rpc);

    MARGO_REGISTER(mid, "unifycr_read_rpc",
                   unifycr_read_in_t, unifycr_read_out_t,
                   unifycr_read_rpc)

    MARGO_REGISTER(mid, "unifycr_mread_rpc",
                   unifycr_mread_in_t, unifycr_mread_out_t,
                   unifycr_mread_rpc);
}

/* margo_server_rpc_init
 *
 * Initialize the server's Margo RPC functionality, for
 * both intra-node (client-server shared memory) and
 * inter-node (server-server).
 */
int margo_server_rpc_init(void)
{
    int rc = UNIFYCR_SUCCESS;

    if (NULL == unifycrd_rpc_context) {
        /* create rpc server context */
        unifycrd_rpc_context = calloc(1, sizeof(ServerRpcContext_t));
        assert(unifycrd_rpc_context);
    }

    margo_instance_id mid;
    mid = setup_sm_target();
    if (mid == MARGO_INSTANCE_NULL) {
        rc = UNIFYCR_FAILURE;
    } else {
        unifycrd_rpc_context->sm_mid = mid;
        register_client_server_rpcs(mid);
    }

#ifdef NOT_YET
    mid = setup_ofi_target();
    if (mid == MARGO_INSTANCE_NULL) {
        rc = UNIFYCR_FAILURE;
    } else {
        unifycrd_rpc_context->ofi_mid = mid;
        register_server_server_rpcs(mid);
    }
#endif

    return rc;
}

/* margo_server_rpc_finalize
 *
 * Finaalize the server's Margo RPC functionality, for
 * both intra-node (client-server shared memory) and
 * inter-node (server-server).
 */
int margo_server_rpc_finalize(void)
{
    int rc = UNIFYCR_SUCCESS;

    if (NULL != unifycrd_rpc_context) {
        /* define a temporary to refer to context */
        ServerRpcContext_t* ctx = unifycrd_rpc_context;
        unifycrd_rpc_context = NULL;

        rpc_clean_server_addr();

        /* shut down margo */
        margo_finalize(ctx->sm_mid);
#ifdef NOT_YET
        margo_finalize(ctx->ofi_mid);
#endif

        /* free memory allocated for context structure */
        free(ctx);
    }

    return rc;
}
