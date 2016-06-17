use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: Check request id pattern
--- config
location = /test {
    requestid on;
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 2: Check server and location both on
--- config
requestid on;

location = /test {
    requestid on;
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 3: Check server directive only
--- config
requestid on;

location = /test {
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 4: Check disabled for location
--- config
requestid on;

location = /test {
    requestid off;
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^$


=== TEST 5: Check location with if
--- config
requestid on;

location = /test {
    if ( $request_method = POST ) {
        return 406;
    }
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 6: Test http context forbidden
--- http_config
requestid on;
--- config
--- must_die


=== TEST 7: No request id by default
--- config
location = /test {
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^$

