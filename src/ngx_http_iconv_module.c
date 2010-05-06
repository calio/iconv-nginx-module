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

    iconv_buf_size = 3;
    max_iconv_bufs = 2560;

    filter.type = NDK_SET_VAR_MULTI_VALUE;
    filter.func = ngx_http_set_iconv_handler;
    filter.size = 3;

    var_name = cf->args->elts;
    var_name++;
    s[0] = *(var_name + 1);

    /* from, to in set_iconv command is case sensitive */
    s[1] = *(var_name + 2);
    if (ngx_strncasecmp((u_char *)"from=", s[1].data, sizeof("from=") - 1)
            != 0) {
        return NGX_CONF_ERROR;
    }
    s[1].data += sizeof("from=") - 1;  /* length of "from=" */

    s[2] = *(var_name + 3);
    if (ngx_strncasecmp((u_char *)"to=", s[2].data, sizeof("to=") - 1) != 0) {
        return NGX_CONF_ERROR;
    }
    s[2].data += sizeof("to=") - 1;  /* length of "to=" */

    return ndk_set_var_multi_value_core(cf, var_name, s, &filter);
}


static ngx_int_t
ngx_http_set_iconv_handler(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v)
{
    ngx_int_t            done;
    ngx_chain_t          chain_head;
    ngx_chain_t         *chain_p;
    ngx_buf_t           *buf;
    iconv_t              cd;
    u_char              *src, *dst, *p;
    size_t               convsize, buf_size, leftlen, outlen;
    u_char *             end;

    dd("%.*s\n%.*s\n%.*s\n", v[0].len, v[0].data, v[1].len, v[1].data, v[2].len, v[2].data);

    if (v->len == 0) {
        res->data = NULL;
        res->len = 0;
        dd("0");
        return NGX_OK;
    }

    src = ngx_palloc(r->pool, v[1].len + 1);
    if (src == NULL) {
        dd("0.1");
        return NGX_ERROR;
    }

    dst = ngx_palloc(r->pool, v[2].len + 1);
    if (dst == NULL) {
        dd("0.2");
        return NGX_ERROR;
    }

    end = ngx_copy(src, v[1].data, v[1].len);
    *end = '\0';
    end = ngx_copy(dst, v[2].data, v[2].len);
    *end = '\0';

    dd("dst:%s\n, src:%s\n", dst, src);
    cd = iconv_open((const char *)dst, (const char *)src);

    if (cd == (iconv_t)-1) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "iconv_open error");
    }

    done = 0;
    chain_p = &chain_head;
    p = v[0].data;
    leftlen = v[0].len;
    outlen = 0;
    while (!done) {
        chain_p->next = ngx_palloc(r->pool, sizeof(ngx_chain_t));
        if (chain_p->next == NULL) {
            return NGX_ERROR;
        }
        dd("1");
        chain_p = chain_p->next;
        buf = ngx_alloc_buf(r->pool);
        chain_p->buf = buf;
        buf->start = ngx_palloc(r->pool, iconv_buf_size);
        if (buf->start == NULL) {
            return NGX_ERROR;
        }
        dd("2");
        buf->pos = buf->start;
        buf->last = buf->pos;
        buf->end = buf->start + iconv_buf_size;

        buf_size = (size_t)iconv_buf_size;

        dd("3");
        convsize = iconv(cd, (char **)&p, &leftlen, (char **)&buf->last,
                &buf_size);
        dd("%.*s\n", buf->last - buf->pos, buf->pos);
        dd("4");
        outlen += (buf->last - buf->pos);
        if(convsize == (size_t)-1 && errno != E2BIG) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "iconv error");
            if (errno == E2BIG) {
                dd("E2BIG");
            } else if (errno == EILSEQ) {
                 dd("EILSEQ");
            } else {
                dd("EINVAL");
            }
            dd("5");
           return NGX_ERROR;
        }
        //buf->last = buf->pos;
        if(leftlen <= 0) {
            dd("6");
            done = 1;
        }
        dd("7");
    }

    iconv_close(cd);

    dd("convert finished");

    res->data = ngx_palloc(r->pool, outlen);
    if(res->data == NULL) {
        return NGX_ERROR;
    }

    p = res->data;
    res->len = outlen;

    for (chain_p = chain_head.next; chain_p; chain_p = chain_p->next) {
        buf = chain_p->buf;
        p = ngx_copy(p, buf->pos, buf->last - buf->pos);
    }
    //fprintf(stderr, "%.*s\n%.*s\n%.*s\n",v[0].len, v[0].data,v[1].len, v[1].data, v[2].len, v[2].data);
    return NGX_OK;
}
