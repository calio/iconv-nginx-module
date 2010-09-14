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



=== TEST 8
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 2;
        echo '106,纪梵希 蜜粉,8,4.5,62.5%
107,时空胶囊,8,3.2857142857143,42.86%
108,雅顿 vc 美白 胶囊,8,7,14.29%
109,水磁场,8,5,14.29%
110,GEL,8,1,100%
111,雅顿 润唇膏 正品,8,8,20%
112,玫瑰面膜,8,1.5,87.5%
113,露得清 祛痘,8,5.1428571428571,42.86%
114,美白水,8,4.75,50%
115,ë,8,1.875,87.5%
116,大米粉,8,1.25,75%
350,薇姿油脂调护洁面啫喱,2,1,100%
475,啫喱 屈臣氏,1,1,100%
569,洗面奶啫喱,1,1,100%';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
106,纪梵希 蜜粉,8,4.5,62.5%
107,时空胶囊,8,3.2857142857143,42.86%
108,雅顿 vc 美白 胶囊,8,7,14.29%
109,水磁场,8,5,14.29%
110,GEL,8,1,100%
111,雅顿 润唇膏 正品,8,8,20%
112,玫瑰面膜,8,1.5,87.5%
113,露得清 祛痘,8,5.1428571428571,42.86%
114,美白水,8,4.75,50%
115,??,8,1.875,87.5%
116,大米粉,8,1.25,75%
350,薇姿油脂调护洁面啫喱,2,1,100%
475,啫喱 屈臣氏,1,1,100%
569,洗面奶啫喱,1,1,100%



=== TEST 9
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 10;
        echo '106,纪梵希 蜜粉,8,4.5,62.5%
107,时空胶囊,8,3.2857142857143,42.86%
108,雅顿 vc 美白 胶囊,8,7,14.29%
109,水磁场,8,5,14.29%
110,GEL,8,1,100%
111,雅顿 润唇膏 正品,8,8,20%
112,玫瑰面膜,8,1.5,87.5%
113,露得清 祛痘,8,5.1428571428571,42.86%
114,美白水,8,4.75,50%
115,ë,8,1.875,87.5%
116,大米粉,8,1.25,75%
350,薇姿油脂调护洁面啫喱,2,1,100%
475,啫喱 屈臣氏,1,1,100%
569,洗面奶啫喱,1,1,100%';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
106,纪梵希 蜜粉,8,4.5,62.5%
107,时空胶囊,8,3.2857142857143,42.86%
108,雅顿 vc 美白 胶囊,8,7,14.29%
109,水磁场,8,5,14.29%
110,GEL,8,1,100%
111,雅顿 润唇膏 正品,8,8,20%
112,玫瑰面膜,8,1.5,87.5%
113,露得清 祛痘,8,5.1428571428571,42.86%
114,美白水,8,4.75,50%
115,??,8,1.875,87.5%
116,大米粉,8,1.25,75%
350,薇姿油脂调护洁面啫喱,2,1,100%
475,啫喱 屈臣氏,1,1,100%
569,洗面奶啫喱,1,1,100%



=== TEST 10
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 2;
        echo 'ë?';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
???



=== TEST 11
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 2;
        echo '?ë';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
???



=== TEST 12
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 3;
        echo '?ë';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
???



=== TEST 13
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 4;
        echo '?ë';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
???



=== TEST 14
--- config
    location /foo {
        iconv_filter from=utf-8 to=gbk;
        iconv_buffer_size 5;
        echo '?ë';
    }
--- request
GET /foo
--- charset: gbk
--- response_body
???
