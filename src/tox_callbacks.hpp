#pragma once

#include <tox/tox.h>

// logging
void log_cb(Tox *tox, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message, void *user_data);

// self
void self_connection_status_cb(Tox *tox, TOX_CONNECTION connection_status, void *user_data);

// friend
//static void friend_name_cb(Tox *tox, uint32_t friend_number, const uint8_t *name, size_t length, void *user_data);
//static void friend_status_message_cb(Tox *tox, uint32_t friend_number, const uint8_t *message, size_t length, void *user_data);
//static void friend_status_cb(Tox *tox, uint32_t friend_number, TOX_USER_STATUS status, void *user_data);
//static void friend_connection_status_cb(Tox *tox, uint32_t friend_number, TOX_CONNECTION connection_status, void *user_data);
//static void friend_typing_cb(Tox *tox, uint32_t friend_number, bool is_typing, void *user_data);
//static void friend_read_receipt_cb(Tox *tox, uint32_t friend_number, uint32_t message_id, void *user_data);
void friend_request_cb(Tox *tox, const uint8_t *public_key, const uint8_t *message, size_t length, void *user_data);
//static void friend_message_cb(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *user_data);

// ngc
void group_peer_name_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, const uint8_t *name, size_t length, void *user_data);
//static void group_peer_status_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, Tox_User_Status status, void *user_data);
//static void group_topic_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, const uint8_t *topic, size_t length, void *user_data);
//static void group_privacy_state_cb(Tox *tox, uint32_t group_number, Tox_Group_Privacy_State privacy_state, void *user_data);
//static void group_voice_state_cb(Tox *tox, uint32_t group_number, Tox_Group_Voice_State voice_state, void *user_data);
//static void group_topic_lock_cb(Tox *tox, uint32_t group_number, Tox_Group_Topic_Lock topic_lock, void *user_data);
//static void group_peer_limit_cb(Tox *tox, uint32_t group_number, uint32_t peer_limit, void *user_data);
//static void group_password_cb(Tox *tox, uint32_t group_number, const uint8_t *password, size_t length, void *user_data);
//static void group_message_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, Tox_Message_Type type, const uint8_t *message, size_t length, uint32_t message_id, void *user_data);
//static void group_private_message_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, Tox_Message_Type type, const uint8_t *message, size_t length, void *user_data);
void group_custom_packet_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length, void *user_data);
void group_custom_private_packet_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length, void *user_data);
void group_invite_cb(Tox *tox, uint32_t friend_number, const uint8_t *invite_data, size_t length, const uint8_t *group_name, size_t group_name_length, void *user_data);
void group_peer_join_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, void *user_data);
void group_peer_exit_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type, const uint8_t *name, size_t name_length, const uint8_t *part_message, size_t length, void *user_data);
void group_self_join_cb(Tox *tox, uint32_t group_number, void *user_data);
//static void group_join_fail_cb(Tox *tox, uint32_t group_number, Tox_Group_Join_Fail fail_type, void *user_data);
//static void group_moderation_cb(Tox *tox, uint32_t group_number, uint32_t source_peer_id, uint32_t target_peer_id, Tox_Group_Mod_Event mod_type, void *user_data);

// ft1

// sha1_info
void ft1_recv_request_sha1_info_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, void* user_data);
bool ft1_recv_init_sha1_info_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size, void* user_data);
void ft1_recv_data_sha1_info_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size, void* user_data);
void ft1_send_data_sha1_info_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size, void* user_data);

// sha1_chunk
void ft1_recv_request_sha1_chunk_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, void* user_data);
bool ft1_recv_init_sha1_chunk_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size, void* user_data);
void ft1_recv_data_sha1_chunk_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size, void* user_data);
void ft1_send_data_sha1_chunk_cb(Tox *tox, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size, void* user_data);

