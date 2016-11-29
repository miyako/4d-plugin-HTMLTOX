4d-plugin-HTMLTOX
=================

4D implementation of wkhtmltopdf. 

###Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🚫|🚫|

###Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

Based on [wkhtmltopdf 0.12.2.3](http://wkhtmltopdf.org/).

###Syntax

```
document:=HTML Convert(sources;format;optionNames;optionValues)
```

param|type|description
------------|------------|----
sources|ARRAY TEXT|URL (``http:``, ``https:``. ``ftp:``, ``file:``), path, HTML
format|INT32|``HTMLTOX Format PDF``, ``HTMLTOX Format PS``, ``HTMLTOX Format PNG``, ``HTMLTOX Format JPG``, ``HTMLTOX Format BMP``. ``HTMLTOX Format SVG``
optionNames|ARRAY TEXT|http://wkhtmltopdf.org/libwkhtmltox/
optionValues|ARRAY TEXT|http://wkhtmltopdf.org/libwkhtmltox/
document|BLOB|PDF or image

###New in 3.0

Callback feature is deprecated. 

You can pass file path, URL or raw HTML as source.

The helper app is launched per process and stays running for the lifetime of that process.

``libwkhtmltox`` is no longer linked as a dylib; instead, a command line program is invoked internally.

**TODO**: ~~Check caller process ID so that multiple instances of 4D can use the plugin.~~ done in ``3.0``

###Examples

* HTML to PDF

```
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
