// derived from url http://neilwallis.com/projects/javascript/codeformat/index.php!

//Language keywords
//var fc_java_kwds="public|int|float|double|private|new|void|synchronized|if|for|byte|break|else";

// Main	
var pres=document.getElementsByTagName("pre");

for (var a=0; a<pres.length; a++) {
  var elem=pres[a];
  if (elem.className.toLowerCase()=='code') formatCode(elem);
}

// Functions
function formatCode(precode) {
  var lang=precode.lang.toLowerCase();
  var textlines=precode.innerHTML.split(/\r|\n/);
  var linecount=1;
  var newcode="";
	  
  // Process each line of text
  for (var b=0; b<textlines.length; b++) {
    var code=textlines[b];

    // Remove line/form feed characters
    code=code.replace(/\f|\n/g,"");

    // Decode special HTML elements
    //code=code.replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;");

    // Double quoted string
    //code=code.replace(/(".+")/g,"<span class=\"quotation\">$1</span>");
    // Single quoted string
    //code=code.replace(/('.+')/g,"<span class=\"quotation\">$1</span>");

    // Make
    if (lang=="make") {
      // comment: make[..] lines
      code=code.replace(/(make\[\d+\]:.*)/,"<span class=\"comment\">$1</span>");
      // emphasized: Making ... in ... lines
      code=code.replace(/(Making \w[\w-]* in \w+)/,"<span class=\"emphasized\">$1</span>");
      // warning: warning lines from gcc
      code=code.replace(/(.*?:\d+:\d+: warning: .*)/,"<span class=\"warning\">$1</span>");
      // error: error lines from gcc
      code=code.replace(/(.*?:\d+:\d+: error: .*)/,"<span class=\"error\">$1</span>");
    }

    // Configure
    if (lang=="configure") {
      // warning: warning lines
      code=code.replace(/(configure: WARNING: .*)/,"<span class=\"warning\">$1</span>");
    }

    // Accumulate line numbers and reformatted text
    var formatline=("   "+linecount).slice(-3);
    newcode+="<span class=\"linenumbers\">"+formatline+"</span>"+code+"<br />";
    linecount++;
  }	  
	  
  // Assign formatted text back to PRE element
  // The outerHTML is used for IE so that
  // whitespace is retained.
  if ("outerHTML" in elem) {
    elem.outerHTML="<pre class='code'><code>"+newcode+"</code></pre>";
  } else {
    elem.innerHTML=newcode;
  }
}

//function colourKeywords(keywords, codeline) {
//  var wordre=new RegExp("("+keywords+") ","gi");
//  return codeline.replace(wordre,"<span class=\"keyword\">$1</span>");
//}

// EOF  
