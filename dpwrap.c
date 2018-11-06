#include <stdio.h>
#include <dplobby.h>
#include "dpwrap.h"
#include "debug.h"

#define CHECK(tag, result) if (result != DP_OK) { printf("%s failed: %s\n", tag, get_error_message(result)); return result; }

void dplobby_message_free(dplobby_message* message) {
  if (message->data != NULL) free(message->data);
  free(message);
}

HRESULT dplobby_create(LPDIRECTPLAYLOBBY3A* out_lobby) {
  LPDIRECTPLAYLOBBY dpli;
  void* convert;

  HRESULT result = DirectPlayLobbyCreate(NULL, &dpli, NULL, NULL, 0);
  CHECK("DirectPlayLobbyCreate", result);

  result = IDirectPlayLobby_QueryInterface(dpli, &IID_IDirectPlayLobby3A, &convert);
  CHECK("QueryInterface", result);

  *out_lobby = (LPDIRECTPLAYLOBBY3A) convert;

  return result;
}

HRESULT dplobby_run_application(LPDIRECTPLAYLOBBY3A lobby, DWORD* app_id, LPDPLCONNECTION connection, HANDLE event) {
  return IDirectPlayLobby_RunApplication(lobby, 0, app_id, connection, event);
}

HRESULT dplobby_receive_message(LPDIRECTPLAYLOBBY3A lobby, DWORD app_id, dplobby_message** out_message) {
  DWORD message_flags = 0;
  DWORD data_size = 0;

  HRESULT result = IDirectPlayLobby_ReceiveLobbyMessage(lobby, 0, app_id, &message_flags, NULL, &data_size);
  if (result != DPERR_BUFFERTOOSMALL) {
    return result;
  }

  message_flags = 0;
  void* data = calloc(1, data_size);
  if (data == NULL) return DPERR_OUTOFMEMORY;
  result = IDirectPlayLobby_ReceiveLobbyMessage(lobby, 0, app_id, &message_flags, data, &data_size);
  CHECK("ReceiveLobbyMessage", result);

  dplobby_message* message = calloc(1, sizeof(dplobby_message));
  message->flags = message_flags;
  message->data = data;
  message->data_size = data_size;

  *out_message = message;
  return DP_OK;
}

HRESULT dpname_create(char* name, LPDPNAME* out_name) {
  LPDPNAME dp_name = calloc(1, sizeof(DPNAME));
  if (dp_name == NULL) return DPERR_OUTOFMEMORY;

  dp_name->dwSize = sizeof(DPNAME);
  dp_name->dwFlags = 0;
  dp_name->lpszShortNameA = name;
  dp_name->lpszLongNameA = name;

  *out_name = dp_name;
  return DP_OK;
}

HRESULT dpsess_create(LPDPSESSIONDESC2* out_session_desc) {
  LPDPSESSIONDESC2 session_desc = calloc(1, sizeof(DPSESSIONDESC2));
  if (session_desc == NULL) return DPERR_OUTOFMEMORY;

  session_desc->dwSize = sizeof(DPSESSIONDESC2);
  session_desc->dwFlags = 0;
  session_desc->guidInstance = GUID_NULL;
  session_desc->guidApplication = GUID_NULL;
  session_desc->dwMaxPlayers = 0;
  session_desc->dwCurrentPlayers = 0;
  session_desc->lpszSessionNameA = "";
  session_desc->lpszPasswordA = "";
  session_desc->dwReserved1 = 0;
  session_desc->dwReserved2 = 0;
  session_desc->dwUser1 = 0;
  session_desc->dwUser2 = 0;
  session_desc->dwUser3 = 0;
  session_desc->dwUser4 = 0;

  *out_session_desc = session_desc;
  return DP_OK;
}

HRESULT dpsess_create_host(GUID application, LPDPSESSIONDESC2* out_session_desc) {
  LPDPSESSIONDESC2 session_desc;
  HRESULT result = dpsess_create(&session_desc);
  CHECK("dpsess_create", result);

  CoCreateGuid(&session_desc->guidInstance);
  session_desc->guidApplication = application;

  *out_session_desc = session_desc;
  return DP_OK;
}

HRESULT dpconn_create(LPDPSESSIONDESC2 session_desc, LPDPNAME player_name, LPDPLCONNECTION* out_connection) {
  LPDPLCONNECTION connection = calloc(1, sizeof(DPLCONNECTION));
  if (connection == NULL) return DPERR_OUTOFMEMORY;

  connection->dwSize = sizeof(DPLCONNECTION);
  connection->dwFlags = 0;
  connection->lpSessionDesc = session_desc;
  connection->lpPlayerName = player_name;
  connection->guidSP = GUID_NULL;
  connection->lpAddress = NULL;
  connection->dwAddressSize = 0;

  *out_connection = connection;
  return DP_OK;
}

void dpconn_set_host(LPDPLCONNECTION connection, char is_host) {
  connection->dwFlags = is_host ? DPLCONNECTION_CREATESESSION : DPLCONNECTION_JOINSESSION;
}

void dpconn_set_service_provider(LPDPLCONNECTION connection, GUID service_provider) {
  connection->guidSP = service_provider;
}
