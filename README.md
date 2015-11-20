# 4d-plugin-HTMLTOX
4D implementation of wkhtmltopdf. 

**CAUTION**: Mac OS only. It seem too difficult (for me!) to call the DLL version of wkhtmltopdf fron 4D; tried MinGW, MSVC, with various options,just seems to crash when the library is called from the main process (used to work before v14; **Maybe the threading model in 4D/Windows has changed?**). It might be better to LAUNCH EXTERNAL PROCESS an executable instead.

###Platform 

OS X 10.6+ 4D 32 bits
OS X 10.8+ 4D 32/64 bits

4D Intel v11 or later.

About
---
Based on [wkhtmltopdf 0.12.2.1](http://wkhtmltopdf.org/) this is an incompatible replacement for [html-converter](https://github.com/miyako/4d-plugin-html-converter)

Examples
---
HTML to PDF
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

HTML to PNG, BMP, JPG, SVG

```
  //the wkhtmltopdf library version is 0.12.2.1, from January 19, 2015
  //http://wkhtmltopdf.org/downloads.html

$path:=Get 4D folder(Current resources folder)+"index.html"

  //3 ways to provide source
ARRAY TEXT($html;1)  //only the first element is used for image
$html{1}:="http://wkhtmltopdf.org/"

  //for full list of keys, see http://wkhtmltopdf.org/libwkhtmltox/
ARRAY TEXT($optionKeys;0)
ARRAY TEXT($optionValues;0)

$resultBlob:=HTML Convert ($html;HTMLTOX Format PNG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".png"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN WEB URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format SVG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".svg"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN WEB URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format BMP;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".bmp"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN WEB URL($path)

$resultBlob:=HTML Convert ($html;HTMLTOX Format JPG;$optionKeys;$optionValues)
$path:=Temporary folder+Generate UUID+".jpg"
BLOB TO DOCUMENT($path;$resultBlob)
OPEN WEB URL($path)
```

Remarks
---
Because Qt (used by wkhtmltopdf) needs to run in the main thread, there is no easy way to implement callbacks; the method runs, but too fact to display or update 4D dialogs, including the PROGRESS component.

Still, for logging purposes you can do the following.

###HTMLTOX_CALLBACK

```
C_LONGINT($1;$intValue)  //for progress, finished
C_LONGINT($2;$processNumber)  //the process that started the conversion; the library itself is running in the main thread
C_LONGINT($3;$progressId)  //if 4d.progressId is passed
C_TEXT($4;$stringValue)  //for warning, error
C_TEXT($5;$callbackType)  //one of the following: progress, warning, error, finished

  //unfortunately the interval is such that the progress bar will not update instantly

$intValue:=$1
$processNumber:=$2
$progressId:=$3
$stringValue:=$4
$callbackType:=$5

Case of 
: ($callbackType="progress")

Progress SET PROGRESS ($progressId;$intValue/100)

: ($callbackType="finished")

Progress SET MESSAGE ($progressId;"complete!")

End case 

$wiD:=Open window(0;0;200;200)
MESSAGE(String($intValue))

CALL PROCESS($processNumber)
```

```
  //the wkhtmltopdf library version is 0.12.2.1, from January 19, 2015
  //http://wkhtmltopdf.org/downloads.html

$path:=Get 4D folder(Current resources folder)+"index.html"

ARRAY TEXT($html;3)
$html{1}:="http://wkhtmltopdf.org/"
$html{2}:="some html source"  //string
$html{3}:=$path

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

$progressId:=Progress New 
Progress SET PROGRESS ($progressId;0)

  //special options for 4d
APPEND TO ARRAY($optionKeys;"4d.progressId")
APPEND TO ARRAY($optionValues;String($progressId))

APPEND TO ARRAY($optionKeys;"4d.callbackMethodName")
APPEND TO ARRAY($optionValues;"HTMLTOX_CALLBACK")

  //callback is called for the following: progress, warning, error, finished
  //however, the interval is such that the progress bar will not update instantly
  //still, you can use it for logging, etc.
$resultBlob:=HTML Convert ($html;HTMLTOX Format PDF;$optionKeys;$optionValues)

Progress QUIT ($progressId)

BLOB TO DOCUMENT(System folder(Desktop)+Generate UUID+".pdf";$resultBlob)
```
