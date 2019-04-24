struct Lpc {
    u16 data_port;
    u16 cmd_port;
};

static struct Lpc lpc_new(void) {
    struct Lpc lpc = {
        .data_port = 0x62,
        .cmd_port = 0x66,
    };
    return lpc;
}

static u8 lpc_sts(struct Lpc * lpc) {
    return inb(lpc->cmd_port);
}

static bool lpc_can_read(struct Lpc * lpc) {
    return (lpc_sts(lpc) & 1) == 1;
}

static int lpc_wait_read(struct Lpc * lpc, int timeout) {
    while (! lpc_can_read(lpc) && timeout > 0) {
        timeout -= 1;
    }
    return timeout;
}

static bool lpc_can_write(struct Lpc * lpc) {
    return (lpc_sts(lpc) & 2) == 0;
}

static int lpc_wait_write(struct Lpc * lpc, int timeout) {
    while (! lpc_can_write(lpc) && timeout > 0) {
        timeout -= 1;
    }
    return timeout;
}

static int lpc_cmd(struct Lpc * lpc, u8 data, int timeout) {
    timeout = lpc_wait_write(lpc, timeout);
    if (timeout > 0) {
        outb(lpc->cmd_port, data);
    }
    return timeout;
}

static int lpc_read(struct Lpc * lpc, u8 * data, int timeout) {
    timeout = lpc_wait_read(lpc, timeout);
    if (timeout > 0) {
        *data = inb(lpc->data_port);
    }
    return timeout;
}

static int lpc_write(struct Lpc * lpc, u8 data, int timeout) {
    timeout = lpc_wait_write(lpc, timeout);
    if (timeout > 0) {
        outb(lpc->data_port, data);
    }
    return timeout;
}
