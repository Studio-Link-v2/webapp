#define SLVERSION "SLVERSION_T"
#ifndef SLPLUGIN
#include <pthread.h>
#include "FLAC/stream_encoder.h"
#endif

#ifdef SLPLUGIN
/*
 * effect/effect.c
 */
void effect_set_bypass(bool value);
#endif

enum ws_type {
	WS_BARESIP,
	WS_CALLS,
	WS_CONTACT,
	WS_CHAT,
	WS_METER,
	WS_CPU,
	WS_OPTIONS,
	WS_RTAUDIO
};

enum webapp_call_state {
	WS_CALL_OFF,
	WS_CALL_RINGING,
	WS_CALL_ON
};

enum {
	DICT_BSIZE = 32,
	MAX_LEVELS = 8,
};

struct webapp {
	struct websock_conn *wc_srv;
	struct le le;
	enum ws_type ws_type;
};

extern enum webapp_call_state webapp_call_status;
extern struct odict *webapp_calls;


char *webapp_provisioning_host(void);

enum sess_chmix {
	CH_STEREO,
	CH_MONO_L,
	CH_MONO_R
};

struct sess_jitter {
	bool talk;
	int16_t bufsz;
	int16_t max;
	int16_t max_l;
	int16_t max_r;
	int16_t startup;
	int16_t silence_count;
	int16_t delay_count;
};

struct session {
	struct le le;
	struct ausrc_st *st_src;
	struct auplay_st *st_play;
	int32_t *dstmix;
	int8_t ch;
	bool run_src;
	bool run_play;
	struct call *call;
	bool stream; /* only used for standalone */
	bool local; /* only used for standalone */
	int8_t track;
	struct sess_jitter jitter;
	enum sess_chmix chmix;
	char *state;
	bool changed;
	size_t buffer;
	double volume;
#ifdef SLPLUGIN
	bool primary;
	bool run_auto_mix;
	bool bypass;
	bool effect_ready;
#else
	bool run_record;
	struct aubuf *aubuf;
	pthread_t record_thread;
	FLAC__StreamEncoder *flac;
	int16_t *sampv;
	FLAC__int32 *pcm;
	float *vumeter;
	FLAC__StreamMetadata *meta[2];
#endif
};

/*
 * sessions.c
 */
void webapp_sessions_init(void);
void webapp_sessions_close(void);
int webapp_session_delete(char * const sess_id, struct call *call);
int8_t webapp_call_update(struct call *call, char *state);
int webapp_session_stop_stream(void);
struct call* webapp_session_get_call(char * const sess_id);
bool webapp_session_available(void);
void webapp_session_chmix(char * const sess_id);
void webapp_session_bufferinc(char * const sess_id);
void webapp_session_bufferdec(char * const sess_id);
void webapp_session_volume(char * const sess_id, char * const volume);

/*
 * account.c
 */
struct ua* webapp_get_main_ua(void);
struct ua* webapp_get_quick_ua(void);
int webapp_accounts_init(void);
void webapp_accounts_close(void);
const struct odict* webapp_accounts_get(void);
void webapp_account_add(const struct odict_entry *acc);
void webapp_account_delete(char *user, char *domain);
void webapp_account_current(void);
void webapp_account_status(const char *aor, bool status);

/*
 * contact.c
 */
int webapp_contacts_init(void);
void webapp_contacts_close(void);
void webapp_contact_add(const struct odict_entry *contact);
void webapp_contact_delete(const char *sip);
bool webapp_contact_exists(const char *sip);
const struct odict* webapp_contacts_get(void);

/*
 * option.c
 */
int webapp_options_init(void);
void webapp_options_close(void);
const struct odict* webapp_options_get(void);
void webapp_options_set(char *key, char *value);
char* webapp_options_getv(char *key, char *def);

/*
 * ws_*.c
 */
void webapp_ws_baresip(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_calls(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_contacts(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_chat(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_meter(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_options(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void ws_meter_process(unsigned int ch, float *in, unsigned long nframes);
void webapp_ws_meter_init(void);
void webapp_ws_meter_close(void);
int webapp_ws_rtaudio_init(void);
void webapp_ws_rtaudio_close(void);
void webapp_ws_rtaudio(const struct websock_hdr *hdr,
		struct mbuf *mb, void *arg);
void webapp_ws_rtaudio_sync(void);

/*
 * websocket.c
 */
void ws_send_all(enum ws_type ws_type, char *str);
void ws_send_all_b(enum ws_type ws_type, struct mbuf *mb);
void ws_send_json(enum ws_type type, const struct odict *o);
void srv_websock_close_handler(int err, void *arg);
int webapp_ws_handler(struct http_conn *conn, enum ws_type type,
		                const struct http_msg *msg,
				websock_recv_h *recvh);
void webapp_ws_init(void);
void webapp_ws_close(void);

/*
 * utils.c
 */
void webapp_odict_add(struct odict *og, const struct odict_entry *e);
int webapp_write_file(char *string, char *filename);
int webapp_write_file_json(struct odict *json, char *filename);
int webapp_load_file(struct mbuf *mb, char *filename);
struct call* webapp_get_call(char *sid);
bool webapp_active_calls(void);

/*
 * slaudio module
 */
const struct odict* slaudio_get_interfaces(void);
void slaudio_record_set(bool status);
void slaudio_record_open_folder(void);
void slaudio_monorecord_set(bool status);
void slaudio_monostream_set(bool status);
void slaudio_mute_set(bool status);
void slaudio_monitor_set(bool status);
int slaudio_reset(void);
void slaudio_set_driver(int value);
void slaudio_set_input(int value);
void slaudio_set_first_input_channel(int value);
void slaudio_set_output(int value);

/*
 * jitter.c
 */
void webapp_jitter_reset(struct session *sess);
void webapp_jitter(struct session *sess, int16_t *sampv,
		auplay_write_h *wh, unsigned int sampc, void *arg);
