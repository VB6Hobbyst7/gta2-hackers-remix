#pragma once
#include "../../common/common.h"
#include "../native.h"
#include <SDL2/SDL_net.h>

typedef struct {
  TCPsocket sock;
  int player_id;
  char player_set;

} net_gta2_session_t;

typedef struct {
  IPaddress ip;
  TCPsocket sock_listen;
  TCPsocket sock_menu;

  // these are sorted by connection time, not by the player
  // id. that's what we have the player_id field inside the
  // struct for.
  net_gta2_session_t *gta2_sessions[GTA2_PLAYER_COUNT];
  int gta2_session_count;

  // Looks like we don't need to re-create this set, once a socket
  // disconnects. Tested with ~10 sessions, although it can hold only
  // 6 of them.
  SDLNet_SocketSet set;
} net_t;

net_t *net_init();

void net_block_until_connected(net_t *net, uint32_t timeout_in_ms);

void net_accept_localhost_only(net_t *net);

void net_frame(net_t *net, native_t *native);

void net_gta2_session_cleanup(net_t *net, int id);

void net_cleanup(net_t *net);

#define NET_SEND_HEADER(SOCKET, HEADER)                                        \
  {                                                                            \
    char header = HEADER;                                                      \
    SDLNet_TCP_Send(SOCKET, &header, 1);                                       \
  }
