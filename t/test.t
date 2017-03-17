use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: Check request id pattern
--- config
location = /test {
    trequestid on;
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 2: Check server and location both on
--- config
trequestid on;

location = /test {
    trequestid on;
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 3: Check server directive only
--- config
trequestid on;

location = /test {
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 4: Check disabled for location
--- config
trequestid on;

location = /test {
    trequestid off;
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^$


=== TEST 5: Check location with if
--- config
trequestid on;

location = /test {
    if ( $request_method = POST ) {
        return 406;
    }
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^[0-9]{13}[0-9a-f]{19}$


=== TEST 6: Test http context forbidden
--- http_config
trequestid on;
--- config
--- must_die


=== TEST 7: No request id by default
--- config
location = /test {
    return 200 "$trequest_id";
}
--- request
GET /test
--- response_body_like: ^$

