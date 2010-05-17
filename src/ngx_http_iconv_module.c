#define DDEBUG 0
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
    ngx_str_t                   *value, s[3];

    iconv_buf_size = 1024;
    max_iconv_bufs = 256;

    filter.type = NDK_SET_VAR_MULTI_VALUE;
    filter.func = ngx_http_set_iconv_handler;
    filter.size = 3;

    value = cf->args->elts;
    value++;
    s[0] = value[1];

    /* 'from', 'to' in set_iconv command is case sensitive */
    s[1] = value[2];
    if (ngx_strncasecmp((u_char *) "from=", s[1].data, sizeof("from=") - 1)
        != 0)
    {
        return NGX_CONF_ERROR;
    }
    s[1].data += sizeof("from=") - 1;

    s[2] = value[3];
    if (ngx_strncasecmp((u_char *) "to=", s[2].data, sizeof("to=") - 1) != 0) {
        return NGX_CONF_ERROR;
    }
    s[2].data += sizeof("to=") - 1;

    return ndk_set_var_multi_value_core(cf, value, s, &filter);
}


static ngx_int_t
ngx_http_set_iconv_handler(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v)
{
    ngx_int_t            done;
    ngx_chain_t          chain_head;
    ngx_chain_t         *cl;
    ngx_buf_t           *buf;
    iconv_t              cd;
    u_char              *src, *dst, *p;
    size_t               size, buf_size, rest, converted;
    u_char *             end;

    if (v->len == 0) {
        res->data = NULL;
        res->len = 0;
        dd("0");
        return NGX_OK;
    }

    src = ngx_palloc(r->pool, v[1].len + 1);
    if (src == NULL) {
        return NGX_ERROR;
    }

    dst = ngx_palloc(r->pool, v[2].len + 1);
    if (dst == NULL) {
        return NGX_ERROR;
    }

    end = ngx_copy(src, v[1].data, v[1].len);
    *end = '\0';
    end = ngx_copy(dst, v[2].data, v[2].len);
    *end = '\0';

    dd("dst:%s\n, src:%s\n", dst, src);
    cd = iconv_open((const char *) dst, (const char *) src);

    if (cd == (iconv_t) -1) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "iconv_open error");
    }

    cl = &chain_head;
    p = v[0].data;
    rest = v[0].len;

    converted = 0;
    done = 0;

    while (!done) {
        cl->next = ngx_palloc(r->pool, sizeof(ngx_chain_t));
        if (cl->next == NULL) {
            return NGX_ERROR;
        }

        cl = cl->next;
        buf = ngx_alloc_buf(r->pool);
        cl->buf = buf;

        buf->start = ngx_palloc(r->pool, iconv_buf_size);
        if (buf->start == NULL) {
            return NGX_ERROR;
        }

        buf->pos = buf->start;
        buf->last = buf->pos;
        buf->end = buf->start + iconv_buf_size;
        buf_size = (size_t) iconv_buf_size;

        size = iconv(cd, (char **) &p, &rest, (char **) &buf->last,
                &buf_size);
        dd("%.*s\n", buf->last - buf->pos, buf->pos);

        if (size == (size_t) -1 && errno != E2BIG) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "iconv error");
            if (errno == E2BIG) {
                dd("E2BIG");
            } else if (errno == EILSEQ) {
                 dd("EILSEQ");
            } else {
                dd("EINVAL");
            }
            return NGX_ERROR;
        }

        converted += buf->last - buf->pos;

        if (rest <= 0) {
            done = 1;
        }
    }

    iconv_close(cd);

    dd("convert finished");

    res->data = ngx_palloc(r->pool, converted);
    if (res->data == NULL) {
        return NGX_ERROR;
    }

    p = res->data;
    res->len = converted;

    for (cl = chain_head.next; cl; cl = cl->next) {
        buf = cl->buf;
        p = ngx_copy(p, buf->pos, buf->last - buf->pos);
    }
    dd("%.*s\n%.*s\n%.*s\n",v[0].len, v[0].data,v[1].len, v[1].data, v[2].len,
            v[2].data);
    return NGX_OK;
}
