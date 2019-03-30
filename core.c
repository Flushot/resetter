#include "core.h"

static int init_zeromq(resetter_context_t *ctx) {
    if (ctx->zmq_port == 0) {
        // Disable zmq: No port specified
        return 0;
    }

    ctx->zmq_ctx = zmq_ctx_new();
    if (ctx->zmq_ctx == NULL) {
        perror("zmq_ctx_new() failed");
        return -1;
    }

    ctx->zmq_pub = zmq_socket(ctx->zmq_ctx, ZMQ_PUB);
    if (ctx->zmq_pub == NULL) {
        perror("zmq_socket() failed");
        return -1;
    }

    char zmq_url[500];
    snprintf(zmq_url, sizeof(zmq_url) / sizeof(char) - 1, "tcp://*:%d", ctx->zmq_port);

    if (zmq_bind(ctx->zmq_pub, zmq_url) == -1) {
        perror("zmq_bind() failed");
        return -1;
    }

    printf("ZeroMQ listening at: %s\n", zmq_url);

    return 0;
}

int core_init(resetter_context_t *ctx) {
    if (init_zeromq(ctx) != 0) {
        core_cleanup(ctx);
        return -1;
    }

    return 0;
}

void core_cleanup(resetter_context_t *ctx) {
    if (ctx->pcap != NULL) {
        pcap_close(ctx->pcap);
        ctx->pcap = NULL;
    }

    if (ctx->libnet != NULL) {
        libnet_destroy(ctx->libnet);
        ctx->libnet = NULL;
    }

    if (ctx->zmq_pub != NULL) {
        zmq_close(ctx->zmq_pub);
        ctx->zmq_pub = NULL;
    }

    if (ctx->zmq_ctx != NULL) {
        zmq_ctx_destroy(ctx->zmq_ctx);
        ctx->zmq_ctx = NULL;
    }
}
