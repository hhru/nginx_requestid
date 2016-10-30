#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>

#define MD5_BHASH_LEN 16
#define MD5_HASH_LEN (MD5_BHASH_LEN * 2)
#define TIMESTAMP_LENGTH 13

typedef struct {
    ngx_flag_t    enable;
} ngx_http_requestid_conf_t;


static ngx_str_t ngx_http_requestid = ngx_string("trequest_id");
static const u_char hex[] = "0123456789abcdef";

static ngx_int_t ngx_http_requestid_add_variables(ngx_conf_t *cf);
static void *ngx_http_requestid_create_conf(ngx_conf_t *cf);
static char *ngx_http_requestid_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);


static ngx_command_t ngx_http_requestid_commands[] = {
    {
        ngx_string("trequestid"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_requestid_conf_t, enable),
        NULL
    },
    ngx_null_command
};


static ngx_http_module_t ngx_http_requestid_module_ctx = {
    ngx_http_requestid_add_variables,      /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_requestid_create_conf,        /* create location configration */
    ngx_http_requestid_merge_conf          /* merge location configration */
};


ngx_module_t ngx_http_requestid_module = {
    NGX_MODULE_V1,
    &ngx_http_requestid_module_ctx,        /* module context */
    ngx_http_requestid_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_requestid_set_variable(ngx_http_request_t *r,
     ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_md5_t                    md5;
    ngx_http_requestid_conf_t    *conf;
    u_char                       *end;
    u_char                       val[NGX_INT64_LEN * 3 + 3];
    u_char                       hashb[MD5_BHASH_LEN];
    ngx_uint_t                   i, k;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_requestid_module);

    if (!conf->enable) {
    	v->not_found = 1;
    	return NGX_OK;
    }

    end = ngx_sprintf(val, "%i,%ui,%i",
                      ngx_pid, r->connection->number, ngx_random());
    *end = 0;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "requestid: data for hash=%s", val);

    ngx_md5_init(&md5);
    ngx_md5_update(&md5, val, end-val);
    ngx_md5_final(hashb, &md5);

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = MD5_HASH_LEN;
    v->data = ngx_pnalloc(r->pool, MD5_HASH_LEN);

    if (v->data == NULL) {
        return NGX_ERROR;
    }

    for (i = TIMESTAMP_LENGTH & ~1, k = 0; i < MD5_HASH_LEN; i += 2, ++k) {
        v->data[i]   = hex[hashb[k] >> 4];
        v->data[i+1] = hex[hashb[k] & 0xf];
    }

    ngx_snprintf(v->data, TIMESTAMP_LENGTH, "%ui%03d",
                 r->start_sec, r->start_msec);

    return NGX_OK;
}


static void *
ngx_http_requestid_create_conf(ngx_conf_t *cf)
{
    ngx_http_requestid_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_requestid_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->enable = NGX_CONF_UNSET;
    return conf;
}


static char *
ngx_http_requestid_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_requestid_conf_t *prev = parent;
    ngx_http_requestid_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_requestid_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t *var;

    var = ngx_http_add_variable(cf, &ngx_http_requestid,
                                NGX_HTTP_VAR_CHANGEABLE);
    if (var == NULL) {
        return NGX_ERROR;
    }

    var->get_handler = ngx_http_requestid_set_variable;

    return NGX_OK;
}
