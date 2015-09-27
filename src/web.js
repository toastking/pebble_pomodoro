//Javascript code for the pomodoror buzz app
//Matthew Del Signore

var regex = /<.[^><]*>|\\/; //regex used for parsing the article
var urls = new Array(); //array of urls to get the buzzfeed articles
var text = ""; //string to send to the watch with article text
var title = ""; //title of the article to be send to the watch


//boilerplate code for an http request
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

//get the urls of news articles
function getUrls(){
  var url = "http://www.buzzfeed.com/api/v2/feeds/news";
   xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object
      var json = JSON.parse(responseText);
      var arr = json.flow; //the array of articles
      
      for(var i = 1; i<15; i++){
        urls.push(arr[i].content.buzz_id);
      }    }      
  );
        console.log(urls);  
 
}

function getArticle(){
  var url = "http://www.buzzfeed.com/api/v2/buzz/" + urls.pop();
  console.log(url);
   var articleText = ""; //the article text
  
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object
      var json = JSON.parse(responseText);
      var arr = json.buzz.subbuzzes; //the array of articles
      
      for(var i=0; i<arr.length;i++){
        if(arr[i].form == "text"){
          var temp = arr[i].description;
          articleText += temp.replace(regex,""); //remove the html tags
        }
      }
    }      
  );
  return articleText;
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    console.log(urls);
    console.log(urls.pop());
    text = getArticle();
    console.log(text);
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);


