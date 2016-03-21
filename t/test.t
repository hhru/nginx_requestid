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
--- server_config
requestid on;
--- config
location = /test {
    requestid on;
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 3: Check server directive only
--- server_config
requestid on;
--- config
location = /test {
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 4: Check disabled for location
--- server_config
requestid on;
--- config
location = /test {
    requestid off;
    return 200 "$request_id";
}
--- request
GET /test
--- response_body_like: ^$


=== TEST 5: Test http context forbidden
--- http_config
requestid on;
--- config
--- must_die

