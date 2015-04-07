<?php
$username='symplecticNDMSGC';
$password='Symplectic14';

//retrieves all groups 
//$URL='https://oxris.ox.ac.uk:8091/elements-api/v4.9/groups';

//retrieve all users of SGC
$URL='https://oxris.ox.ac.uk:8091/elements-api/v4.9/users?groups=38&per-page=200';

//retrieve susanne's fav publications
//$URL='https://oxris.ox.ac.uk:8091/elements-api/v4.9/relationships?involving=user(609)&types=8&page=1&detail=ref&per-page=999'; 
//retrieve susanne's publications
//$URL='https://oxris.ox.ac.uk:8091/elements-api/v4.9/relationships?involving=user(609)&types=8&detail=full&page=2'; 

//to test
//$URL='https://oxris.ox.ac.uk:8091/elements-api/v4.9/relationships?involving=user(8482)&types=8&page=1&detail=ref&per-page=999';



$ch = curl_init();
curl_setopt($ch, CURLOPT_URL,$URL);
curl_setopt($ch, CURLOPT_TIMEOUT, 30); //timeout after 30 seconds
curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
curl_setopt($ch, CURLOPT_USERPWD, "$username:$password");
$status_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);   //get status code
print"ooo";
$result=curl_exec ($ch);
print($result);
file_put_contents('sources/members.xml', $result); 

curl_close ($ch);

?>
