# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

#repeat_each(3);

plan tests => repeat_each() * 2 * blocks();

no_long_string();

run_tests();

#no_diff();

__DATA__

=== TEST 1
--- config
    location /foo {
        set $src '你 好';
        set_iconv $dst $src from=utf8 to=gbk;
        echo $dst;
    }
--- request
GET /foo
--- charset: gbk
--- response_body
你 好


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
