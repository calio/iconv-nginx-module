#define DDEBUG 1
#include "ddebug.h"

#include <ndk.h>
#include <iconv.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t       iconv_buf_size;
static ngx_int_t       max_iconv_bufs;


static char * ngx_http_set_iconv_conf_handler(ngx_conf_t *cf,
        ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_set_iconv_handler(ngx_http_request_t *r,
        ngx_str_t *res, ngx_http_variable_value_t *v);


static ngx_command_t ngx_http_iconv_commands[] = {

    { ngx_string("set_iconv"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE4,
      ngx_http_set_iconv_conf_handler,
      0,
      0,
      NULL
    },

    ngx_null_command
};


static ngx_http_module_t ngx_http_iconv_module_ctx = {
    NULL,                   /* preconfiguration */
    NULL,                   /* postconfiguration */
    NULL,                   /* create main configuration */
    NULL,                   /* init main configuration */
    NULL,                   /* create server configuration */
    NULL,                   /* merge server configuration */
    NULL,                   /* create location configuration */
    NULL                    /* merge location configuration */
};


ngx_module_t ngx_http_iconv_module = {
    NGX_MODULE_V1,
    &ngx_http_iconv_module_ctx,     /* module context */
    ngx_http_iconv_commands,        /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init maseter */
    NULL,                           /* init module */
    NULL,                           /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    NULL,                           /* exit process */
    NULL,                           /* exit master */
    NGX_MODULE_V1_PADDING
};


static char *
ngx_http_set_iconv_conf_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ndk_set_var_t                filter;
    ngx_str_t                   *var_name, s[3];

    iconv_buf_size = 1000;
    max_iconv_bufs = 256;

    filter.type = NDK_SET_VAR_MULTI_VALUE;
    filter.func = ngx_http_set_iconv_handler;
    filter.size = 3;

    var_name = cf->args->elts;
    var_name++;
    s[0] = *var_name;

    /* from, to in set_iconv command is case sensitive */
    s[1] = *(var_name + 1);
    if (ngx_strncasecmp((u_char *)"from=", s[1].data, 5) != 0) {
        return NGX_CONF_ERROR;
    }
    s[1].data += 5;  /* length of "from=" */

    s[2] = *(var_name + 2);
    if (ngx_strncasecmp((u_char *)"to=", s[2].data, 5) != 0) {
        return NGX_CONF_ERROR;
    }
    s[2].data += 3;  /* length of "to=" */

    return ndk_set_var_multi_value_core(cf, var_name, s, &filter);
}

static ngx_int_t
ngx_http_set_iconv_handler(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v)
{
    ngx_int_t           done;
    ngx_chain_t         chain;
    ngx_chain_t         *chain_p;
    ngx_buf_t           *buf;

    if (v->len == 0) {
        res->data = 0;
        res->len = 0;
        return NGX_OK;
    }

    done = 0;

    while (!done) {
        buf = ngx_palloc(r->pool, sizeof(iconv_buf_t));

        buf->data = ngx_palloc(r->pool, iconv_buf_size);
        buf->len = iconv_buf_size;

    }

    fprintf(stderr, "%.*s\n%.*s\n%.*s\n",v[0].len, v[0].data,v[1].len, v[1].data, v[2].len, v[2].data);
    dd("test debug");
    return NGX_OK;
}
