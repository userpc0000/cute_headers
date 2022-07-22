#define CUTE_NET_IMPLEMENTATION
#include "../../cute_net.h"

#include <time.h>

// This can be whatever you want. It's a unique identifier for your game or application, and
// used merely to identify yourself within Cute's packets. It's more of a formality than anything.
uint64_t g_application_id = 0;

// These keys were generated by the below function `print_embedded_keygen`. Usually it's a good idea
// to distribute your keys to your servers in absolute secrecy, perhaps keeping them hidden in a file
// and never shared publicly. But, putting them here is fine for testing purposes.

// Embedded g_public_key
int g_public_key_sz = 32;
unsigned char g_public_key_data[32] = {
	0x4a,0xc5,0x56,0x47,0x30,0xbf,0xdc,0x22,0xc7,0x67,0x3b,0x23,0xc5,0x00,0x21,0x7e,
	0x19,0x3e,0xa4,0xed,0xbc,0x0f,0x87,0x98,0x80,0xac,0x89,0x82,0x30,0xe9,0x95,0x6c
};
// Embedded g_secret_key
int g_secret_key_sz = 64;
unsigned char g_secret_key_data[64] = {
	0x10,0xaa,0x98,0xe0,0x10,0x5a,0x3e,0x63,0xe5,0xdf,0xa4,0xb5,0x5d,0xf3,0x3c,0x0a,
	0x31,0x5d,0x6e,0x58,0x1e,0xb8,0x5b,0xa4,0x4e,0xa3,0xf8,0xe7,0x55,0x53,0xaf,0x7a,
	0x4a,0xc5,0x56,0x47,0x30,0xbf,0xdc,0x22,0xc7,0x67,0x3b,0x23,0xc5,0x00,0x21,0x7e,
	0x19,0x3e,0xa4,0xed,0xbc,0x0f,0x87,0x98,0x80,0xac,0x89,0x82,0x30,0xe9,0x95,0x6c
};

uint64_t unix_timestamp() {
	time_t ltime;
	time(&ltime);
	struct tm* timeinfo = gmtime(&ltime);;
	return (uint64_t)mktime(timeinfo);
}

void panic(cn_error_t err) {
	printf("ERROR: %s\n", err.details);
	exit(-1);
}

// Get this header from here: https://github.com/RandyGaul/cute_headers/blob/master/cute_time.h
#define CUTE_TIME_IMPLEMENTATION
#include "../../cute_time.h"

int main(void) {
	const char* address_and_port = "127.0.0.1:5001";
	cn_endpoint_t endpoint;
	cn_endpoint_init(&endpoint, address_and_port);

	cn_server_config_t server_config = cn_server_config_defaults();
	server_config.application_id = g_application_id;
	memcpy(server_config.public_key.key, g_public_key_data, sizeof(g_public_key_data));
	memcpy(server_config.secret_key.key, g_secret_key_data, sizeof(g_secret_key_data));

	cn_server_t* server = cn_server_create(server_config);
	cn_error_t err = cn_server_start(server, address_and_port);
	if (cn_is_error(err)) panic(err);
	printf("Server started, listening on port %d.\n", (int)endpoint.port);

	while (1) {
		float dt = ct_time();
		uint64_t unix_time = unix_timestamp();

		cn_server_update(server, (double)dt, unix_time);

		cn_server_event_t e;
		while (cn_server_pop_event(server, &e)) {
			if (e.type == CN_SERVER_EVENT_TYPE_NEW_CONNECTION) {
				printf("New connection from id %d, on index %d.\n", (int)e.u.new_connection.client_id, e.u.new_connection.client_index);
			} else if (e.type == CN_SERVER_EVENT_TYPE_PAYLOAD_PACKET) {
				printf("Got a message from client on index %d, \"%s\"\n", e.u.payload_packet.client_index, (const char*)e.u.payload_packet.data);
				cn_server_free_packet(server, e.u.payload_packet.client_index, e.u.payload_packet.data);
			} else if (e.type == CN_SERVER_EVENT_TYPE_DISCONNECTED) {
				printf("Client disconnected on index %d.\n", e.u.disconnected.client_index);
			}
		}
	}

	cn_server_destroy(server);

	return 0;
}
