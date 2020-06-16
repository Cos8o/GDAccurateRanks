<?php

//Endpoint return values [GD: value (GDBrowser API)]

//1 = userName (username)
//2 = userID (playerID)
//3 = stars
//4 = demons
//6 = rank
//7 = extid (0)
//8 = creatorPoints (cp)
//9 = icon (NA)
//10 = color1 (NA)
//11 = color2 (NA)
//13 = coins
//14 = iconType (NA)
//15 = special (NA)
//16 = extid (0)
//17 = userCoins (usercoins)
//46 = diamonds

//Endpoints

$userEndpoint = "https://www.gdbrowser.com/api/profile/";
$scoresEndpoint = "https://www.gdbrowser.com/api/leaderboard?accurate";
$gdEndpoint = "http://www.boomlings.com/database/getGJScores20.php";

//Values from post request

$gameVersion = $_POST["gameVersion"];
$binaryVersion = $_POST["binaryVersion"];
$gdw = $_POST["gdw"];
$accountID = $_POST["accountID"];
$gjp = $_POST["gjp"];
$type = $_POST["type"];
$count = $_POST["count"];
$secret = $_POST["secret"];

//Helpers

function doGet($url)
{
	$c = curl_init();

	curl_setopt($c, CURLOPT_URL, $url);
	curl_setopt($c, CURLOPT_RETURNTRANSFER, true); 

	$response = curl_exec($c);
    
	curl_close($c);
	return $response;
}

function doPost($url, $data)
{
	$c = curl_init();

	curl_setopt($c, CURLOPT_URL, $url);
	curl_setopt($c, CURLOPT_POST, true);
	curl_setopt($c, CURLOPT_POSTFIELDS, $data);
	curl_setopt($c, CURLOPT_RETURNTRANSFER, true); 

	$response = curl_exec($c);
    
	curl_close($c);
	return $response;
}

//Handler

if (!strcmp($type, "top"))
{
	$response = "";
    
	//Attempt request to GDBrowser's API
    
	$buffer = doGet($scoresEndpoint);
	$rankData = json_decode($buffer);
    
    	//Iterate thru all players
    
    	foreach($rankData as $player)
    	{
    		//I mean, it's top 100 tbh
    		if ($player->rank == 101)
			break;
    
    		$buffer = doGet($userEndpoint.$player->playerID);
        	$playerData = json_decode($buffer);
    
    		$response .= "1:".$player->username;
        	$response .= ":2:".$player->playerID;
		$response .= ":3:".$player->stars;
		$response .= ":4:".$player->demons;
		$response .= ":6:".$player->rank;
		$response .= ":7:"."0";
		$response .= ":8:".$player->cp;
		$response .= ":9:".$playerData->icon;
        	$response .= ":10:".$playerData->col1;
        	$response .= ":11:".$playerData->col2;
		$response .= ":13:".$player->coins;
		$response .= ":14:0"; //TBA (iconType)
		$response .= ":15:".$playerData->glow;
		$response .= ":16:".$playerData->accountID;
		$response .= ":17:".$player->usercoins;
        	$response .= ":46:".$player->diamonds;
        
        	$response .= "|";
	}
    
	echo substr($response, 0, -1);
}
else
{
	//Post request to GD server
    
    	$request = "gameVersion=".$gameVersion;
    	$request .= "&binaryVersion=".$binaryVersion;
    	$request .= "&gdw=".$gdw;
    	$request .= "&accountID=".$accountID;
    	$request .= "&gjp=".$gjp;
	$request .= "&type=".$type;
	$request .= "&count=".$count;
	$request .= "&secret=".$secret;
    
	echo doPost($gdEndpoint, $request);
}

?>
