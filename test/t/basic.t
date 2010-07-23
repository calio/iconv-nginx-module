# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

#repeat_each(3);

plan tests => repeat_each() * 2 * blocks();

#no_long_string();

run_tests();

#no_diff();

__DATA__

=== TEST 1
--- config
    location /foo {
        set $src '你好';
        set_iconv $dst $src from=utf-8 to=gbk;
        echo $dst;
    }
--- request
GET /foo
--- charset: gbk
--- response_body
你好


=== TEST 2
--- config
    location /foo {
        set $src '淘宝网 - 亚洲最大、最安全的网上交易平台，提供各类服饰、美容、家居、数码、话费/点卡充值… 2亿优质特价商品，同时提供担保交易(先收货后付款)、先行赔付、假一赔三、七天无理由退换货、数码免费维修等安全交易保障服务，让你全面安心享受网上购物乐趣！';
        set_iconv $dst $src from=UTF-8 to=gbk;
        echo $dst;
    }
--- request
GET /foo
--- charset: gbk
--- response_body
淘宝网 - 亚洲最大、最安全的网上交易平台，提供各类服饰、美容、家居、数码、话费/点卡充值… 2亿优质特价商品，同时提供担保交易(先收货后付款)、先行赔付、假一赔三、七天无理由退换货、数码免费维修等安全交易保障服务，让你全面安心享受网上购物乐趣！


=== TEST 3
--- config
    location /foo {
        set $src '你好';
        iconv_buffer_size 1M;
        set_iconv $dst $src from=utf-8 to=GBK;
        echo $dst;
    }
--- request
GET /foo
--- charset: gbk
--- response_body
你好

=== TEST 4
--- config
    location /foo {
        iconv_filter from=utf-8 to=GBK;
        echo '你好';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
你好


=== TEST 5
--- config
    location /foo {
        iconv_filter from=utf-8 to=GBK;
        #set_unescape_uri $a $arg_a;
        #set_unescape_uri $b $arg_b;
        #set_unescape_uri $c $arg_c;
        #set_unescape_uri $d $arg_d;
        echo -n $arg_a;
        echo -n $arg_b;
        echo -n $arg_c;
        echo  $arg_d;
    }
--- request
GET /foo?a=你&b=好&c=世&d=界
--- charset: gbk
--- response_body
你好世界


=== TEST 6
--- config
    location /foo {
        iconv_filter from=utf-8 to=GBK;
        set_unescape_uri $a $arg_a;
        set_unescape_uri $b $arg_b;
        set_unescape_uri $c $arg_c;
        set_unescape_uri $d $arg_d;
        set_unescape_uri $e $arg_e;
        set_unescape_uri $f $arg_f;
        echo -n $a;
        echo -n $b;
        echo -n $c;
        echo -n $d;
        echo -n $e;
        echo  $f;
    }
--- request
GET /foo?a=%e4&b=%bd&c=%a0&d=%e5&e=%a5&f=%bd
--- charset: gbk
--- response_body
你好


=== TEST 7
--- config
    location /foo {
        iconv_filter from=utf-8 to=GBK;
        iconv_buffer_size 1k;
        set_unescape_uri $a $arg_a;
        set_unescape_uri $b $arg_b;
        set_unescape_uri $c $arg_c;
        set_unescape_uri $d $arg_d;
        set_unescape_uri $e $arg_e;
        set_unescape_uri $f $arg_f;
        echo -n $a;
        echo -n $b;
        echo -n $c;
        echo -n $d;
        echo -n $e;
        echo  $f;
    }
--- request
GET /foo?a=%e4&b=%bd&c=%a0&d=%e5&e=%a5&f=%bd
--- charset: gbk
--- response_body
你好
