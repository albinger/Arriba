function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    console.log("options not sent to Pebble: " + e.error.message);
}

Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Figure out current platform
    var platform, watchinfo;
    if(Pebble.getActiveWatchInfo) {
      watchinfo= Pebble.getActiveWatchInfo();
      platform=watchinfo.platform;
    } else {
      platform="aplite";
    } 
    //Load the remote config page
    Pebble.openURL("https://dl.dropboxusercontent.com/u/10052545/arriba1_07.html?watch_info="+platform);
  }
);

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("configuration closed");
    if (e.response != '') {
	var options = JSON.parse(decodeURIComponent(e.response));
	console.log("storing options: " + JSON.stringify(options));
	window.localStorage.setItem('options', JSON.stringify(options));
	Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
    } else {
	console.log("no options received");
    }
});