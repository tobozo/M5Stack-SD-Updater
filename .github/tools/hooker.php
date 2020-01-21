<?php

/*

Bash usage example:

  php hooker.php [Hook URL] [Event Name] [JSON Code] [Hook Secret]

  php hooker.php "${hookURL}" "${eventName}" "${jsonCode}" "${hookSecret}";
  if [ $? -eq 0 ]; then
    echo "yay";
  else
    echo "nay";
  fi

*/


function usage( $msg ) {
  if( trim( $msg ) !='' ) {
    echo "\n$msg\n";
  }
  echo "\n[Hooker Usage]\n";
  //echo '    php hooker.php "${hookURL}" "${eventName}" "${jsonCode}" "${hookSecret}"'."\n";
  echo "    php hooker.php [Hook URL] [Event Name] [JSON Code] [Hook Secret]\n";
  echo "    php ".basename(__FILE__).' "https://my.site/hook" "my-custom-event" \'{"action":"custom","data":["blah","bleb","foo","bar"]}\''." \"My Secret pass\"\n\n";
}


if( basename( $argv[0] ) !== basename( __FILE__ ) ) {
  usage( "[ERROR] API Mode only !!");
  exit(1);
}

if( !isset($argv[1]) || !isset($argv[2]) || !isset($argv[3]) || !isset($argv[4]) ) {
  usage( "[ERROR] Missing arg");
  exit(1);
}

$hookURL    = trim( $argv[1] );
$eventName  = trim( $argv[2] );
$payload    = trim( $argv[3] );
$hookSecret = trim( $argv[4] );

if( empty( $payload ) || empty ( $eventName ) || empty( $hookSecret ) || empty( $hookURL ) ) {
  usage( "[ERROR] Empty arg" );
  exit(1);
}

$json = json_decode( $payload, true );

if(! $json ) {
  usage("\n[ERROR] Payload has invalid json: $payload\n\n");
  exit(1);
}
if( !isset( $json['action'] ) ) {
  usage("\n[ERROR] Payload has no action: $payload\n\n");
  exit(1);
}
if( trim( $json['action'] ) == '' ) {
  usage("\n[ERROR] Payload has empty action: $payload\n\n");
  exit(1);
}
// turn full path + empty data into basename + b64 encoded data
if( isset( $json['base_name'] ) && file_exists( $json['base_name'] ) ) {
  $file_path = $json['base_name'];
  $json['data'] = 'fakedata';
  $json['base_name'] = basename( $json['base_name'] );
  echo "file $json[base_name] is reachable :-)\n";
} else {
  echo "file $json[base_name] is not reachable :-(\n";
}

// re-encode
$payload = json_encode( $json );
$algo = 'sha256'; // go strong :-)

$rawPost = 'payload='.$payload; // simulate rawPost for GH hook compliance
$rawPostSize = strlen( $payload );
$hashed = hash_hmac($algo, $rawPost, $hookSecret);

$headers = sprintf( '-H "Content-Type: multipart/form-data" -H "User-Agent: PHP-M5-Registry-UA" -H "X-Github-Event: %s" -H "X-Hub-Signature: %s=%s"',
  $eventName,
  $algo,
  $hashed
);
$cmd = "curl -X POST $headers -F 'payload=$payload' -F binary=@\"$file_path\" $hookURL";

echo "[Hooker CMD]\n\n\n\n\n".$cmd."\n\n\n\n\n";
exec( $cmd, $out );
echo "[Hook RESPONSE]\n\n\n\n\n".implode("\n", $out)."\n\n\n\n\n"; exit;

$context = stream_context_create( $opts );
$response = file_get_contents( $hookURL, false, $context );

if( ! $response ) {
  usage( "[ERROR] Hooker cannot post comment (size: $rawPostSize): bad gh api response\n\n" );
  exit(1);
}

if( !preg_match("/200/", $http_response_header[0]) ) {
  usage( "[ERROR] Hooker got bad response code from post (size: $rawPostSize): $response / ".print_r($http_response_header, 1) );
  exit(1);
} else {
  echo json_encode([ 'headers' => $http_response_header, 'response_body' => $response ]);
  exit(0);
}


