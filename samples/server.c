#include <stdio.h>
#include <pandio.h>
#include <string.h>

int counter = 1;

void handle_write(pd_write_t *write_op, int status) {
    //printf("Written successfully with status: %d\n", status);
    free(write_op->data.buf);
    free(write_op);
}

void handle_read(pd_tcp_t *client, char *buf, size_t len) {
    //printf("Received data with len: %zu\n", len);
    printf("%.*s\n", (int)len, buf);
    free(buf);
    const char *response = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 5\r\n\r\nHello";
    pd_write_t *write_op = malloc(sizeof(pd_write_t));
    pd_write_init(write_op, strdup(response), strlen(response), handle_write);

    pd_tcp_write(client, write_op);
    pd_tcp_shutdown(client);
}

void handle_close(pd_tcp_t *client) {
    printf("Connection closed\n");
    free(client);
}

void handle_connection(pd_tcp_server_t *server, pd_socket_t socket, int status) {
    pd_tcp_t *client = malloc(sizeof(pd_tcp_t));
    pd_tcp_init(server->ctx, client);
    client->on_data = handle_read;
    client->on_close = handle_close;

    pd_tcp_accept(client, socket);
    //printf("Received connection #%d\n", counter++);
}

int main() {
    pd_io_t *ctx = malloc(sizeof(pd_io_t));
    pd_io_init(ctx);

    pd_tcp_server_t *server = malloc(sizeof(pd_tcp_server_t));
    pd_tcp_server_init(ctx, server);
    printf("Starting to listen...\n");
    int status = pd_tcp_listen(server, 8000, handle_connection);
    if (status < 0) {
        printf("Listener failed.\n");
    }

    pd_io_run(ctx);
}
