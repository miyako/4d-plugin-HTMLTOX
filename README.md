# 4d-plugin-HTMLTOX
4D implementation of wkhtmltopdf. 

##Releases

[64 bit](https://github.com/miyako/4d-plugin-HTMLTOX/releases/tag/0.12.3)

##New

Passing raw HTML is no longer supported. Only a URL or system path should be passed.

``libwkhtmltox`` is no longer linked as a dylib; instead, a command line program is invoked internally.

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🚫|🚫|

About
---
Based on [wkhtmltopdf 0.12.2.3](http://wkhtmltopdf.org/).

Commands
---

```c
HTML_Convert
```

Examples
---

* HTML to PDF

```
  //the wkhtmltopdf library version is 0.12.2.1, from January 19, 2015
  //http://wkhtmltopdf.org/downloads.html

$path:=Get 4D folder(Database folder)+\
"Plugins"+Folder separator+\
"HTMLTOX.bundle"+Folder separator+\
"Contents"+Folder separator+\
"Resources"+Folder separator+\
"Documentation"+Folder separator+\
"index.html"

  //3 ways to provide source
ARRAY TEXT($html;3)
$html{1}:="http://wkhtmltopdf.org/"  //a: URL
$html{2}:=Document to text($path)  //b: HTML source code
$html{3}:=$path  //c: path of HTML

  //for full list of keys, see http://wkhtmltopdf.org/libwkhtmltox/
ARRAY TEXT($optionKeys;0)
ARRAY TEXT($optionValues;0)
  //special consideration: for per-object (page) settings, 
  //prefix the key with an element number corresponding to the array of htmls.
  //e.g. "size.paperSize" this is a global setting
  //"1:toc.useDottedLines" this is a per-object setting for $html{1}

APPEND TO ARRAY($optionKeys;"size.paperSize")
APPEND TO ARRAY($optionValues;"A4")

APPEND TO ARRAY($optionKeys;"orientation")
APPEND TO ARRAY($optionValues;"Landscape")

$resultBlob:=HTML Convert ($html;HTMLTOX Format PDF;$optionKeys;$optionValues)

$path:=Temporary folder+Generate UUID+".pdf"

BLOB TO DOCUMENT($path;$resultBlob)

OPEN WEB URL($path)
```

* HTML to PNG, BMP, JPG, SVG

```
ARRAY TEXT($html;1)  //only the first element is used for image
$html{1}:="http://wkhtmltopdf.org/"

  //for full list of keys, see http://wkhtmltopdf.org/libwkhtmltox/
ARRAY TEXT($optionKeys;0)
ARRAY TEXT($optionValues;0)

$resultBlob:=HTML Convert ($html;HTMLTOX Format PNG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".png"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format SVG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".svg"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format BMP;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".bmp"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format JPG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".jpg"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN URL($path)
```

* Notification

```
C_LONGINT($1;$intValue)  //for progress, finished
C_LONGINT($2;$processNumber)  //the process that started the conversion; the library itself is running in the main thread
C_LONGINT($3;$progressId)  //if 4d.progressId is passed
C_TEXT($4;$stringValue)  //for warning, error
C_TEXT($5;$callbackType)  //one of the following: progress, warning, error, finished

$intValue:=$1
$processNumber:=$2
$progressId:=$3
$stringValue:=$4
$callbackType:=$5

Case of 
	: ($callbackType="progress")
		
		Progress SET PROGRESS ($progressId;$intValue/100)
		
	: ($callbackType="finished")
		
		If ($intValue=1)
			Progress SET MESSAGE ($progressId;"complete!")
		End if 
		
	: ($callbackType="warning")
		
	: ($callbackType="error")
		
End case 
```

* Usage with callback

```
ARRAY TEXT($optionKeys;0)
ARRAY TEXT($optionValues;0)

$progressId:=Progress New 
Progress SET PROGRESS ($progressId;0)

ARRAY TEXT($html;3)
$html{1}:=Get 4D folder(Current resources folder)+"sample1.html"
$html{2}:=Get 4D folder(Current resources folder)+"sample1.html"
$html{3}:=Get 4D folder(Current resources folder)+"sample1.html"

  //global
APPEND TO ARRAY($optionKeys;"size.paperSize")
APPEND TO ARRAY($optionValues;"A4")
APPEND TO ARRAY($optionKeys;"orientation")
APPEND TO ARRAY($optionValues;"Landscape")

  //special options for 4d
APPEND TO ARRAY($optionKeys;"4d.progressId")
APPEND TO ARRAY($optionValues;String($progressId))

APPEND TO ARRAY($optionKeys;"4d.callbackMethodName")
APPEND TO ARRAY($optionValues;"HTMLTOX_CALLBACK")

$resultBlob:=HTML Convert ($html;HTMLTOX Format PDF;$optionKeys;$optionValues)

Progress QUIT ($progressId)

If (BLOB size($resultBlob)#0)
	
	$dstPath:=Temporary folder+Generate UUID+".pdf"
	
	BLOB TO DOCUMENT($dstPath;$resultBlob)
	
	OPEN URL($dstPath)
	
End if 
```
